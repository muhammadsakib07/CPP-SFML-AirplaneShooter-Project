#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdlib>
#include <ctime>
//hi i am nayeem islam
enum class AppState { MENU, PLAY, HIGHSCORE, THEMES, EXIT };

//Bullet Structure 
struct Bullet {
    sf::Sprite sprite;
    float speed = 12.f;   

    Bullet(const sf::Vector2f& pos, sf::Texture& texture) {
        sprite.setTexture(texture);
        sprite.setScale(0.7f, 0.7f);
        sprite.setPosition(pos);
    }

    void update() { sprite.move(0.f, -speed); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};


//Enemy Structure
struct Enemy {
    sf::Sprite sprite;
    float speed = 3.f;

    Enemy(const sf::Vector2f& pos, sf::Texture& texture) {
        sprite.setTexture(texture);
        sprite.setPosition(pos);
        sprite.setScale(0.7f, 0.7f);
    }

    void update() { sprite.move(0.f, speed); }
    sf::FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};


int main(){
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    //Window
    sf::RenderWindow window(sf::VideoMode(800, 600), "Save the Earth: Alien War");
    window.setFramerateLimit(60);

    //Assets
    sf::Font font;
    if (!font.loadFromFile("../assets/fonts/arcade.ttf")) {
        std::cerr << "Failed to load font\n"; return -1;
    }

    sf::Texture bgTex;
    if (!bgTex.loadFromFile("../assets/images/bg.png")) {
        std::cerr << "Failed to load bg\n";
        return -1;
    }   
    sf::Sprite bgSprite(bgTex);
    bgSprite.setScale(window.getSize().x / float(bgTex.getSize().x),
                      window.getSize().y / float(bgTex.getSize().y));
           
    sf::Texture Bgoption1;
    if (!bgTex.loadFromFile("../assets/images/bg.png")) {
        std::cerr << "Failed to load bg\n";
        return -1;
    }  
    sf::Texture Bgoption2;
    if (!bgTex.loadFromFile("../assets/images/bg.png")) {
        std::cerr << "Failed to load bg\n";
        return -1;
    }  
    sf::Texture Bgoption3;
    if (!bgTex.loadFromFile("../assets/images/bg.png")) {
        std::cerr << "Failed to load bg\n";
        return -1;
    }  

    sf::Texture enemyTexture;
        if (!enemyTexture.loadFromFile("../assets/images/enemy7.png")) {
            std::cerr << "Failed to load enemy image!\n"; return -1;
        }

        sf::Texture fireTexture;
        if (!fireTexture.loadFromFile("../assets/images/fire.png")) {
            std::cerr << "Failed to load fire bullet image!\n";
            return -1;
        }

    sf::Music bgMusic;
    if (!bgMusic.openFromFile("../assets/sounds/intro.wav")) std::cerr << "Failed to load music\n";
    else { bgMusic.setLoop(true); bgMusic.play(); }

    sf::SoundBuffer selectBuf;
    sf::Sound selectSound;
    if (selectBuf.loadFromFile("../assets/sounds/laser.wav")) selectSound.setBuffer(selectBuf);

    //Menu
    std::vector<std::string> menuItems = {"Play Game", "High Score", "Themes", "Exit"};
    std::vector<sf::Text> menuTexts(menuItems.size());
    sf::RectangleShape highlight(sf::Vector2f(420.f, 48.f));
    highlight.setFillColor(sf::Color(255,255,255,60));
    highlight.setOrigin(highlight.getSize().x/2, highlight.getSize().y/2);

    for (size_t i = 0; i < menuItems.size(); i++) {
        menuTexts[i].setFont(font);
        menuTexts[i].setString(menuItems[i]);
        menuTexts[i].setCharacterSize(36);
        menuTexts[i].setFillColor(sf::Color::White);
        sf::FloatRect tb = menuTexts[i].getLocalBounds();
        menuTexts[i].setOrigin(tb.left + tb.width/2, tb.top + tb.height/2);
        menuTexts[i].setPosition(window.getSize().x/2.f, 220.f + i*55.f);
    }

    int selected = 0;
    AppState state = AppState::MENU;
    int highScore = 0;

    //Player setup
    sf::Texture playerTexture;
    if (!playerTexture.loadFromFile("../assets/images/plane0.png")) {
    std::cerr << "Failed to load player image!\n";
    return -1;
    }

    sf::Sprite playerSprite;
    playerSprite.setTexture(playerTexture);
    playerSprite.setPosition(400, 425);
    playerSprite.setScale(2.0f, 2.0f);
    float playerSpeed = 6.f;


    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    sf::Clock enemySpawnClock;


    //Game loop
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();

            if (event.type == sf::Event::KeyPressed) {
                if (state == AppState::MENU) {
                    if (event.key.code == sf::Keyboard::Up) selected = (selected - 1 + menuItems.size()) % menuItems.size();
                    if (event.key.code == sf::Keyboard::Down) selected = (selected + 1) % menuItems.size();
                    if (event.key.code == sf::Keyboard::Enter) {
                        switch(selected) {
                            case 0: state = AppState::PLAY; break;
                            case 1: state = AppState::HIGHSCORE; break;
                            case 2: state = AppState::THEMES; break;
                            case 3: window.close(); break;
                        }
                        if (selectBuf.getSampleCount() > 0) selectSound.play();
                    }
                } else if (state == AppState::PLAY) {
                    if (event.key.code == sf::Keyboard::Escape) state = AppState::MENU;
                    if (event.key.code == sf::Keyboard::Space) {
                    sf::Vector2f bulletPos;
                    bulletPos.x = playerSprite.getPosition().x + playerSprite.getGlobalBounds().width / 2 - 3.f; 
                    bulletPos.y = playerSprite.getPosition().y - 10.f; 
                    bullets.emplace_back(bulletPos, fireTexture);
}

                } else if (state == AppState::HIGHSCORE || state == AppState::THEMES) {
                    if (event.key.code == sf::Keyboard::Escape) state = AppState::MENU;
                }
            }
        }

        //Menu highlight
        if (state == AppState::MENU) highlight.setPosition(window.getSize().x/2.f, 220.f + selected*55.f);

        //Player controls
        if (state == AppState::PLAY) {
           if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) && playerSprite.getPosition().x > 0) 
    playerSprite.move(-playerSpeed, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) && playerSprite.getPosition().x + playerSprite.getGlobalBounds().width < window.getSize().x) 
    playerSprite.move(playerSpeed, 0.f);


            // Spawn enemies using the shared texture
            if (enemySpawnClock.getElapsedTime().asSeconds() > 1.f) {
                enemies.push_back(Enemy(sf::Vector2f(rand() % (window.getSize().x - 50), 0.f), enemyTexture));
                enemySpawnClock.restart();
            }


            // Update bullets
            for (auto& b : bullets) b.update();
           bullets.erase(std::remove_if(bullets.begin(), bullets.end(),
    [&](Bullet& b){ return b.sprite.getPosition().y < 0; }), bullets.end());

            // Update enemies
            for (auto& e : enemies) e.update();
            enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
                        [&](Enemy& e){ return e.sprite.getPosition().y > window.getSize().y; }), enemies.end());

            // Check collisions
            for (auto it = enemies.begin(); it != enemies.end();) {
                bool destroyed = false;
                for (auto bt = bullets.begin(); bt != bullets.end();) {
                    if (it->getBounds().intersects(bt->sprite.getGlobalBounds())) {
                        bt = bullets.erase(bt);
                        destroyed = true;
                        highScore += 10;
                        break;
                    } else ++bt;
                }
                if (destroyed) it = enemies.erase(it);
                else ++it;
            }
        }

        //Draw
        window.clear();
        window.draw(bgSprite);

        if (state == AppState::MENU) {
            window.draw(highlight);
            for (auto& t : menuTexts) window.draw(t);
        } else if (state == AppState::PLAY) {
            window.draw(playerSprite);
            for (auto& b : bullets) window.draw(b.sprite);
            for (auto& e : enemies) window.draw(e.sprite);

            sf::Text scoreText("Score: " + std::to_string(highScore), font, 20);
            scoreText.setPosition(10, 10);
            window.draw(scoreText);
        } else if (state == AppState::HIGHSCORE) {
            sf::Text hsText("High Score: " + std::to_string(highScore), font, 40);
            hsText.setPosition(250, 250);
            window.draw(hsText);
        }

        window.display();
    }
    return 0;
}