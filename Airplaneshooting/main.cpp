#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>

enum class AppState { MENU, PLAY, THEMES, HIGHSCORE, ABOUT, BACKGROUND_MENU, SHIPSKIN_MENU, EXIT };


//Bullet struct
 struct Bullet {
    sf::Sprite sprite;
    float speed = 700.f;
    bool active = true;

    Bullet(const sf::Vector2f& pos, sf::Texture& texture) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x/2.f, texture.getSize().y/2.f);
        sprite.setScale(0.7f, 0.7f);
        sprite.setPosition(pos);
    }

    void update(float dt) {
        sprite.move(0.f, -speed * dt);
        if (sprite.getPosition().y < -50.f) active = false;
    }

    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};


// Enemy struct
struct Enemy {
    sf::Sprite sprite;
    float speed = 80.f;
    bool alive = true;
    int type = 0; 

    Enemy(const sf::Vector2f& pos, sf::Texture& texture, float s = 80.f, int t = 0) {
        sprite.setTexture(texture);
        sprite.setOrigin(texture.getSize().x/2.f, texture.getSize().y/2.f);

        if (t == 0) sprite.setScale(0.7f, 0.7f);    
        else if (t == 1) sprite.setScale(0.5f, 0.5f); 
        else if (t == 2) sprite.setScale(0.9f, 0.9f); 

        sprite.setPosition(pos);
        speed = s;
        type = t;
    }

    void update(float dt) {
        sprite.move(0.f, speed * dt);
        if (sprite.getPosition().y > 900.f) alive = false;
    }

    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};





int main() {
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    const unsigned int WIN_W = 1100, WIN_H = 900;
    sf::RenderWindow window(sf::VideoMode(WIN_W, WIN_H), "Save the Earth: Alien War");
    window.setFramerateLimit(120);



    //Load assets
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/arcade.ttf")) {
        std::cerr << "Warning: failed to load font at assets/fonts/arcade.ttf. HUD will use default if available.\n";
    }


    // Background default
    sf::Texture bgDefaultTex;
    if (!bgDefaultTex.loadFromFile("../assets/images/bg.png")) {
        std::cerr << "Warning: failed to load assets/images/bg.png — continuing with plain background.\n";
    }

    sf::Sprite bgSprite;
    if (bgDefaultTex.getSize().x > 0) {
        bgSprite.setTexture(bgDefaultTex);
        bgSprite.setScale(
            float(window.getSize().x) / float(bgDefaultTex.getSize().x),
            float(window.getSize().y) / float(bgDefaultTex.getSize().y)
        );
    }


    // Theme textures : three options
    sf::Texture themeTex[3];
    bool themeLoaded[3] = {false,false,false};
    themeLoaded[0] = themeTex[0].loadFromFile("../assets/images/option1BG.jpg");
    themeLoaded[1] = themeTex[1].loadFromFile("../assets/images/option2BG.jpg");
    themeLoaded[2] = themeTex[2].loadFromFile("../assets/images/option3BG.jpg");
    for (int i=0;i<3;i++) {
        if (!themeLoaded[i]) {
            std::cerr << "Theme " << (i+1) << " not found at assets/images/option" << (i+1) << "BG.png\n";
        }
    }

    // Ship skins : 3 options
    sf::Texture shipSkins[3];
    bool shipLoaded[3] = {false, false, false};
    shipLoaded[0] = shipSkins[0].loadFromFile("../assets/images/plane0.png");
    shipLoaded[1] = shipSkins[1].loadFromFile("../assets/images/plane1.png");
    shipLoaded[2] = shipSkins[2].loadFromFile("../assets/images/plane2.png");
    for(int i=0;i<3;i++) {
        if(!shipLoaded[i]) std::cerr << "Ship skin " << i << " not found!\n";
    }
    int activeShip = 0; 


    sf::Text backgroundOption, shipSkinsOption;
    int themeMenuIndex = 0;

    backgroundOption.setFont(font);
    backgroundOption.setString("Background");
    backgroundOption.setCharacterSize(40);
    backgroundOption.setPosition(WIN_W/2 - 100, WIN_H/2 - 50);

    shipSkinsOption.setFont(font);
    shipSkinsOption.setString("Ship Skins");
    shipSkinsOption.setCharacterSize(40);
    shipSkinsOption.setPosition(WIN_W/2 - 100, WIN_H/2 + 20);


    // Player texture
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("../assets/images/plane0.png")) {
        std::cerr << "Warning: player texture not found at assets/images/plane0.png — using fallback shape.\n";
    }


    sf::Texture heartTexture;
    bool haveHeartTex = heartTexture.loadFromFile("../assets/images/hp1.png");


    // Bullet and enemy textures
    sf::Texture bulletTexture;
    sf::Texture enemyTextures[3];
    bool haveEnemyTex[3];
    haveEnemyTex[0] = enemyTextures[0].loadFromFile("../assets/images/enemy7.png");
    haveEnemyTex[1] = enemyTextures[1].loadFromFile("../assets/images/enemy6.png");
    haveEnemyTex[2] = enemyTextures[2].loadFromFile("../assets/images/enemy9.png");

    bool haveBulletTex = bulletTexture.loadFromFile("../assets/images/fire.png");
    if (!haveBulletTex) std::cerr << "No bullet image at assets/images/fire.png\n";


    sf::Texture backTex;
    bool haveBackTex = false;
    sf::Sprite backSprite;
    sf::RectangleShape backRect;              


    if (backTex.loadFromFile("../assets/images/back.png")) {
        haveBackTex = true;
        backSprite.setTexture(backTex);
        const float desiredW = 48.f;
        float s = desiredW / float(backTex.getSize().x);
        backSprite.setScale(s, s);
        backSprite.setPosition(16.f, 16.f);
    }


        // Sounds
        sf::SoundBuffer selectBuf, shootBuf, explBuf;
        sf::Sound selectSound, shootSound, explSound;
        if (selectBuf.loadFromFile("../assets/sounds/laser.wav")) selectSound.setBuffer(selectBuf);
        if (shootBuf.loadFromFile("../assets/sounds/shoot.wav")) shootSound.setBuffer(shootBuf);
        if (explBuf.loadFromFile("../assets/sounds/explosion.wav")) explSound.setBuffer(explBuf);

        sf::Music bgMusic;
        if (bgMusic.openFromFile("../assets/sounds/intro.wav")) {
            bgMusic.setLoop(true);
            bgMusic.play();
        }

    
    std::vector<std::string> menuItems = {"Play Game", "High Score", "Themes", "About", "Exit"};
    std::vector<sf::Text> menuTexts(menuItems.size());
    for (size_t i = 0; i < menuItems.size(); ++i) {
        menuTexts[i].setFont(font);
        menuTexts[i].setString(menuItems[i]);
        menuTexts[i].setCharacterSize(36);
        menuTexts[i].setFillColor(sf::Color::White);
        sf::FloatRect tb = menuTexts[i].getLocalBounds();
        menuTexts[i].setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
        menuTexts[i].setPosition(window.getSize().x/2.f, 200.f + i * 60.f);
    }


    std::vector<std::string> themeItems = {"Background", "Ship Skins"};
    std::vector<sf::Text> themeTexts(themeItems.size());

    for (size_t i = 0; i < themeItems.size(); ++i) {
        themeTexts[i].setFont(font);
        themeTexts[i].setString(themeItems[i]);
        themeTexts[i].setCharacterSize(36);
        themeTexts[i].setFillColor(sf::Color::White);
        sf::FloatRect tb = themeTexts[i].getLocalBounds();
        themeTexts[i].setOrigin(tb.left + tb.width/2.f, tb.top + tb.height/2.f);
        themeTexts[i].setPosition(window.getSize().x/2.f, 200.f + i * 60.f);
    }



    sf::RectangleShape highlight(sf::Vector2f(460.f, 56.f));
    highlight.setFillColor(sf::Color(255,255,255,60));
    highlight.setOrigin(highlight.getSize().x/2.f, highlight.getSize().y/2.f);


    std::vector<sf::Sprite> themeSprites;
    for (int i=0;i<3;i++) {
        if (themeLoaded[i]) {
            sf::Sprite s(themeTex[i]);
            float scale = 160.f / float(themeTex[i].getSize().x); 
            s.setScale(scale, scale * float(themeTex[i].getSize().x) / float(themeTex[i].getSize().y)); 
            themeSprites.push_back(s);
        } else {
            sf::RenderTexture rt; rt.create(160, 100);
            rt.clear(sf::Color(40 + i*40, 80 + i*30, 120 + i*20));
            rt.display();
            sf::Sprite s(rt.getTexture());
            themeSprites.push_back(s);
        }
        
        themeSprites.back().setPosition(120.f + i * 220.f, 200.f);
    }


        std::vector<sf::Sprite> shipSprites;
        for(int i=0;i<3;i++){
            if(shipLoaded[i]){
                sf::Sprite s(shipSkins[i]);
                float scale = 100.f / float(shipSkins[i].getSize().x);
                s.setScale(scale, scale * float(shipSkins[i].getSize().x)/float(shipSkins[i].getSize().y));
                shipSprites.push_back(s);
            } else {
                sf::RenderTexture rt; rt.create(100, 60);
                rt.clear(sf::Color(100+i*30, 50+i*20, 200));
                rt.display();
                shipSprites.push_back(sf::Sprite(rt.getTexture()));
            }
            shipSprites.back().setPosition(120.f + i*220.f, 350.f); // below themes
        }



  
    sf::Sprite playerSprite;
    playerSprite.setTexture(playerTexture);
    float targetWidth = 140.f;  
    float scaleX = targetWidth / playerTexture.getSize().x; 
    float scaleY = scaleX; 
    playerSprite.setScale(scaleX, scaleY); 
    playerSprite.setOrigin(playerTexture.getSize().x / 2.f, playerTexture.getSize().y / 2.f); 
    playerSprite.setPosition(WIN_W/2.f, WIN_H - 80.f); 
    float playerSpeed = 420.f; 

    
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    sf::Clock enemySpawnClock;

 
    AppState state = AppState::MENU;
    int selected = 0;
    int highScore = 0;
    int activeTheme = -1;
    bool activeThemeSelected = true; 

    sf::Text hud;
    hud.setFont(font);
    hud.setCharacterSize(18);
    hud.setFillColor(sf::Color::White);
    hud.setPosition(8.f, 8.f);


    sf::Clock clock;
     
    sf::Texture fallbackBulletTex;
    bool haveFallbackBullet = false;
    if (!haveBulletTex) {
        sf::RenderTexture rt;
        rt.create(6,12);
        rt.clear(sf::Color::Transparent);
        sf::RectangleShape r({6.f, 12.f});
        r.setFillColor(sf::Color::Yellow);
        r.setPosition(0.f, 0.f);
        rt.draw(r);
        rt.display();
        fallbackBulletTex = rt.getTexture(); 
        haveFallbackBullet = true;
    }

    sf::Texture fallbackEnemyTex;
    bool haveFallbackEnemy = false;
    if (!haveEnemyTex) {
        sf::RenderTexture rt2; rt2.create(44,32);
        rt2.clear(sf::Color::Transparent);
        sf::RectangleShape r2({44.f, 32.f});
        r2.setFillColor(sf::Color(240,80,80));
        r2.setPosition(0.f, 0.f);
        rt2.draw(r2);
        rt2.display();
        fallbackEnemyTex = rt2.getTexture();
        haveFallbackEnemy = true;
    }

    int playerLives = 4;


    // Main loop
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        //Events 
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) { window.close(); break; }

    if (event.type == sf::Event::KeyPressed) {
    //keyboard handling
    if (state == AppState::MENU) {
        if (event.key.code == sf::Keyboard::Up) {
            selected = (selected - 1 + (int)menuItems.size()) % (int)menuItems.size();
        } else if (event.key.code == sf::Keyboard::Down) {
            selected = (selected + 1) % (int)menuItems.size();
        } else if (event.key.code == sf::Keyboard::Enter) {
            if (selected == 0) {
                state = AppState::PLAY;
            } else if (selected == 1) {
                state = AppState::HIGHSCORE;
            } else if (selected == 2) {
                if (activeTheme < 0 || activeTheme >= 3) activeTheme = 0;
                state = AppState::THEMES;
            } else if (selected == 3) {
                state = AppState::ABOUT;
            } else if (selected == 4) {
                window.close();
            }
            if (selectSound.getBuffer()) selectSound.play();
        }
    }
    else if (state == AppState::PLAY) {
        if (event.key.code == sf::Keyboard::Escape) {
            state = AppState::MENU;
        } else if (event.key.code == sf::Keyboard::Space) {
            sf::Vector2f bulletPos = playerSprite.getPosition() - sf::Vector2f(0.f, playerSprite.getGlobalBounds().height/2.f + 8.f);
            if (haveBulletTex) bullets.emplace_back(bulletPos, bulletTexture);
            else if (haveFallbackBullet) bullets.emplace_back(bulletPos, fallbackBulletTex);
            if (shootSound.getBuffer()) shootSound.play();
        }
    }
    else if (state == AppState::HIGHSCORE) { 
        if (event.key.code == sf::Keyboard::Escape) state = AppState::MENU;
    } 
   else if (state == AppState::BACKGROUND_MENU) {
        if (event.key.code == sf::Keyboard::Left) {
            activeTheme = (activeTheme - 1 + 3) % 3;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Right) {
            activeTheme = (activeTheme + 1) % 3;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Enter) {
            if (activeTheme >= 0 && activeTheme < 3 && themeLoaded[activeTheme]) {
                bgSprite.setTexture(themeTex[activeTheme], true);
                bgSprite.setScale(
                    float(window.getSize().x) / float(themeTex[activeTheme].getSize().x),
                    float(window.getSize().y) / float(themeTex[activeTheme].getSize().y)
                );
            }
            state = AppState::MENU;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Escape) {
            state = AppState::MENU;
        }
    }
    else if (state == AppState::SHIPSKIN_MENU) {
        if (event.key.code == sf::Keyboard::Left) {
            activeShip = (activeShip - 1 + 3) % 3;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Right) {
            activeShip = (activeShip + 1) % 3;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Enter) {
          if (activeShip >= 0 && activeShip < 3 && shipLoaded[activeShip]) {
    playerSprite.setTexture(shipSkins[activeShip], true);

    float targetWidth = 140.f;   // desired width in pixels
    float scaleX = targetWidth / shipSkins[activeShip].getSize().x;
    float scaleY = scaleX;
    playerSprite.setScale(scaleX, scaleY);
    playerSprite.setOrigin(shipSkins[activeShip].getSize().x / 2.f, shipSkins[activeShip].getSize().y / 2.f);
}


            state = AppState::MENU;
            if (selectSound.getBuffer()) selectSound.play();
        } else if (event.key.code == sf::Keyboard::Escape) {
            state = AppState::MENU;
        }
    }
}
//mouse handling
else if (event.type == sf::Event::MouseButtonPressed) {
    if (event.mouseButton.button == sf::Mouse::Left) {
    sf::Vector2f mpos((float)event.mouseButton.x, (float)event.mouseButton.y);


    if (state == AppState::THEMES || state == AppState::HIGHSCORE || state == AppState::ABOUT || state == AppState::BACKGROUND_MENU || state == AppState::SHIPSKIN_MENU) {
        bool clickedBack = false;
        if (haveBackTex) {
            if (backSprite.getGlobalBounds().contains(mpos)) clickedBack = true;
        } else {
            if (backRect.getGlobalBounds().contains(mpos)) clickedBack = true;
        }
        if (clickedBack) {
            state = AppState::MENU;
            if (selectSound.getBuffer()) selectSound.play();
            continue; 
        }
    }

    if (state == AppState::MENU) {
         if (state == AppState::MENU) {
            for (size_t i = 0; i < menuTexts.size(); ++i) {
                if (menuTexts[i].getGlobalBounds().contains(mpos)) {
                    selected = (int)i;
                    if (selected == 0) {
                        state = AppState::PLAY;
                    } else if (selected == 1) {
                        state = AppState::HIGHSCORE;
                    } else if (selected == 2) {
                        if (activeTheme < 0 || activeTheme >= 3) activeTheme = 0;
                        state = AppState::THEMES;
                    } else if (selected == 3) {
                        state = AppState::ABOUT;
                    } else if (selected == 4) {
                        window.close();
                    }
                    if (selectSound.getBuffer()) selectSound.play();
                    break;
                }
            }
        }
    }

    else if (state == AppState::THEMES) {
    sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

            // Assuming you have sf::Text or sf::RectangleShape for your menu items
            if (backgroundOption.getGlobalBounds().contains(mousePos)) {
                state = AppState::BACKGROUND_MENU;
            } 
            else if (shipSkinsOption.getGlobalBounds().contains(mousePos)) {
                state = AppState::SHIPSKIN_MENU;
            } 
    }


   else if (state == AppState::PLAY) {
            sf::Vector2f bulletPos = playerSprite.getPosition() - sf::Vector2f(0.f, playerSprite.getGlobalBounds().height/2.f + 8.f);
            if (haveBulletTex) bullets.emplace_back(bulletPos, bulletTexture);
            else if (haveFallbackBullet) bullets.emplace_back(bulletPos, fallbackBulletTex);
            if (shootSound.getBuffer()) shootSound.play();
        }
    else if (state == AppState::HIGHSCORE) {
        state = AppState::MENU;
    }
    else if (state == AppState::BACKGROUND_MENU) {
       sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);

            // Click on theme sprites
            for (int i = 0; i < 3; i++) {
                if (themeSprites[i].getGlobalBounds().contains(mousePos)) {
                    activeTheme = i;  // select this theme
                    if (themeLoaded[activeTheme]) {
                        bgSprite.setTexture(themeTex[activeTheme], true);
                        bgSprite.setScale(
                            float(window.getSize().x) / float(themeTex[activeTheme].getSize().x),
                            float(window.getSize().y) / float(themeTex[activeTheme].getSize().y)
                        );
                    }
                    state = AppState::MENU;
                    if (selectSound.getBuffer()) selectSound.play();
                    break;
                }
            }
    }
    else if (state == AppState::SHIPSKIN_MENU) {
       sf::Vector2f mousePos(event.mouseButton.x, event.mouseButton.y);


            for (int i = 0; i < 3; i++) {
                if (shipSprites[i].getGlobalBounds().contains(mousePos)) {
                    activeShip = i;  
                      if (activeShip >= 0 && activeShip < 3 && shipLoaded[activeShip]) {
                        playerSprite.setTexture(shipSkins[activeShip], true);
                        float targetWidth = 140.f;   
                        float scaleX = targetWidth / shipSkins[activeShip].getSize().x;
                        float scaleY = scaleX;
                        playerSprite.setScale(scaleX, scaleY);
                        playerSprite.setOrigin(shipSkins[activeShip].getSize().x / 2.f, shipSkins[activeShip].getSize().y / 2.f);
                    }
                    
                    state = AppState::MENU;
                    if (selectSound.getBuffer()) selectSound.play();
                    break;
                }
            }
    }
}

}

else if (event.type == sf::Event::MouseMoved) {
    sf::Vector2f mpos((float)event.mouseMove.x, (float)event.mouseMove.y);
    if (state == AppState::MENU) {
        for (size_t i = 0; i < menuTexts.size(); ++i) {
            if (menuTexts[i].getGlobalBounds().contains(mpos)) {
                selected = (int)i;
                break;
            }
        }
    }
}
} 

        //Update
        if (state == AppState::MENU) {
            highlight.setPosition(window.getSize().x/2.f, 200.f + selected * 60.f);
        } else if (state == AppState::THEMES) {
           highlight.setPosition(window.getSize().x/2.f, 200.f + selected * 60.f);
        } else if (state == AppState::PLAY) {
            
            if (activeTheme >= 0 && themeLoaded[activeTheme]) {
                bgSprite.setTexture(themeTex[activeTheme]);
                bgSprite.setScale(
                    float(window.getSize().x) / float(themeTex[activeTheme].getSize().x),
                    float(window.getSize().y) / float(themeTex[activeTheme].getSize().y)
                );
            }

            // movement left/right
            sf::Vector2f moveDir(0.f, 0.f);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A)) moveDir.x -= 1.f; 
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) moveDir.x += 1.f; 
            if (moveDir.x != 0.f) { 
                playerSprite.move(moveDir * playerSpeed * dt); 
            }
            sf::FloatRect pb = playerSprite.getGlobalBounds();
            sf::Vector2f ppos = playerSprite.getPosition();
            float halfw = pb.width/2.f, halfh = pb.height/2.f;
            if (ppos.x < halfw) ppos.x = halfw;
            if (ppos.x > window.getSize().x - halfw) ppos.x = window.getSize().x - halfw;
            playerSprite.setPosition(ppos);

            
            if (enemySpawnClock.getElapsedTime().asSeconds() > 1.2f) {
                int count = 1;
                for (int i = 0; i < count; i++) {
                float x = 40.f + std::rand() % (window.getSize().x - 80);

                int type = std::rand() % 3; 

                float sp = 0.f;
                if (type == 0) sp = 80.f + std::rand() % 40; 
                else if (type == 1) sp = 100.f + std::rand() % 30;
                else if (type == 2) sp = 70.f + std::rand() % 20;

                if (haveEnemyTex[type]) {
                    enemies.emplace_back(sf::Vector2f(x, -30.f), enemyTextures[type], sp, type);
        } 
    }
    enemySpawnClock.restart();
}



            // update bullets and enemies
            for (auto &b : bullets) b.update(dt);
            bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& bb){ return !bb.active; }), bullets.end());

            for (auto &e : enemies) e.update(dt);
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& ee){ return !ee.alive; }), enemies.end());

            // collision bullets vs enemies
            for (auto &b : bullets) {
                if (!b.active) continue;
                for (auto &e : enemies) {
                    if (!e.alive) continue;
                    if (b.getBounds().intersects(e.getBounds())) {
                        b.active = false;
                        e.alive = false;
                        if (explBuf.getSampleCount()>0) explSound.play();
                        highScore += 10;
                        break;
                    }
                }
            }

} 

        // Render
        window.clear();

 
        if (bgSprite.getTexture()) {
            window.draw(bgSprite);
        } else {
            window.clear(sf::Color(10,14,28));
        }

        if (state == AppState::MENU) {
            window.draw(highlight);
            for (auto &t : menuTexts) window.draw(t);
        } else if (state == AppState::THEMES) {
            if (haveBackTex) window.draw(backSprite);
             window.draw(backgroundOption);
             window.draw(shipSkinsOption);

         if (themeMenuIndex == 0) backgroundOption.setFillColor(sf::Color::Yellow);
         else backgroundOption.setFillColor(sf::Color::White);

        if (themeMenuIndex == 1) shipSkinsOption.setFillColor(sf::Color::Yellow);
        else shipSkinsOption.setFillColor(sf::Color::White);
        } else if (state == AppState::PLAY) {
            window.draw(playerSprite);
            for (auto &b : bullets) window.draw(b.sprite);
            for (auto &e : enemies) if (e.alive) window.draw(e.sprite);

            hud.setString("Score: " + std::to_string(highScore));
            hud.setScale(1.6f,1.6f);
            window.draw(hud);
          for (int i = 0; i < playerLives; ++i) { 
            if (haveHeartTex) { 
                sf::Sprite heart; 
                heart.setTexture(heartTexture); 
                heart.setScale(1.2f, 1.2f); 
                heart.setPosition(10.f + i * 30.f, 40.f);
                window.draw(heart); 
            }
        }
    } else if (state == AppState::HIGHSCORE) {
            sf::Text hs("High Score: " + std::to_string(highScore), font, 36);
            hs.setFillColor(sf::Color::White);
            hs.setPosition(200.f, 260.f);
            if (haveBackTex) window.draw(backSprite);
            window.draw(hs);
        }
        else if (state == AppState::ABOUT) {
        sf::Text aboutTitle("About - How to Play", font, 32);
        aboutTitle.setFillColor(sf::Color::Yellow);
        aboutTitle.setPosition(180.f, 60.f);
        window.draw(aboutTitle);

        std::string instructions =
            "Controls:\n"
            "- Move Left:  Left Arrow / A\n"
            "- Move Right: Right Arrow / D\n"
            "- Shoot:      Spacebar or Left Mouse Click\n"
            "- Pause/Exit: Esc\n\n"
            "Gameplay:\n"
            "- Your mission is to defend the Earth by destroying enemy ships.\n"
            "- Each enemy destroyed gives +10 points.\n"
            "- Survive as long as possible and try to beat your high score!\n";

        sf::Text aboutText(instructions, font, 20);
        aboutText.setFillColor(sf::Color::White);
        aboutText.setPosition(100.f, 140.f);
        window.draw(aboutText);
       if (haveBackTex) window.draw(backSprite);
    } else if(state == AppState::BACKGROUND_MENU) {
           // Draw ship skin thumbnails            
            sf::Text t("Choose Theme (Click or press 1/2/3)", font, 24);
            t.setFillColor(sf::Color::White);
            t.setPosition(120.f, 120.f);
            window.draw(t);

            // draw thumbnails with labels
            for (int i=0;i<3;i++) {
                window.draw(themeSprites[i]);
                sf::Text lab(std::to_string(i+1), font, 20);
                lab.setFillColor(sf::Color::White);
                lab.setPosition(themeSprites[i].getPosition().x + themeSprites[i].getGlobalBounds().width/2.f - 8.f,
                                themeSprites[i].getPosition().y + themeSprites[i].getGlobalBounds().height + 8.f);
                window.draw(lab);
            }

            sf::Text back("Esc to return", font, 16);
            back.setFillColor(sf::Color(200,200,200));
            back.setPosition(10.f, WIN_H - 28.f);
            window.draw(back);
            if (haveBackTex) window.draw(backSprite);


    }  else if(state == AppState::SHIPSKIN_MENU) {  
    sf::Text shipLabel("Choose Ship Skin", font, 24);  
    shipLabel.setFillColor(sf::Color::White);  
    shipLabel.setPosition(window.getSize().x / 2.f - shipLabel.getGlobalBounds().width / 2.f, 50.f); // top center 
    window.draw(shipLabel);  

    float spacing = 50.f; 
    float totalWidth = 0.f; 
    
    for(int i = 0; i < 3; i++){  
        float scaleX = 150.f / shipSprites[i].getTexture()->getSize().x; 
        float scaleY = scaleX; 
       shipSprites[i].setOrigin(shipSkins[i].getSize().x/2.f, shipSkins[i].getSize().y/2.f);
       shipSprites[i].setScale(scaleX, scaleY);
        totalWidth += shipSprites[i].getGlobalBounds().width; 
        if(i < 2) totalWidth += spacing; 
    } 

    float startX = window.getSize().x / 2.f - totalWidth / 2.f; 
    float yPos = 250.f; 

    for(int i = 0; i < 3; i++){  
        shipSprites[i].setPosition(startX + shipSprites[i].getGlobalBounds().width / 2.f, yPos); 
        window.draw(shipSprites[i]);  
        startX += shipSprites[i].getGlobalBounds().width + spacing; 
    }  
    if (haveBackTex) window.draw(backSprite);  
} 


     window.display();

    // save highscore
    if (highScore > 0) {
        std::ofstream out("highscore.txt");
        if (out) out << highScore;
    }
}
return 0;
}
