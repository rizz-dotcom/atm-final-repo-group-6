#define _CRT_SECURE_NO_WARNINGS
#include<SFML/Graphics.hpp>
#include<iostream>
#include<ctime>
#include<fstream>
#include<sstream>
using namespace std;

// ================= BUTTON CLASS =================
class Button
{
public:
    sf::RectangleShape shape;
    sf::Text text;
    bool isHovered;

    Button() : isHovered(false) {}

    Button(float x, float y, float width, float height, const string& label, const sf::Font& font)
        : isHovered(false)
    {
        shape.setSize(sf::Vector2f(width, height));
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::Blue);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);

        text.setFont(font);
        text.setString(label);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color::White);

        float textX = x + (width - text.getLocalBounds().width) / 2;
        float textY = y + (height - text.getLocalBounds().height) / 2 - 5;
        text.setPosition(textX, textY);
    }

    bool isClicked(sf::Vector2f mousePos)
    {
        return shape.getGlobalBounds().contains(mousePos);
    }

    void update(sf::Vector2f mousePos)
    {
        isHovered = shape.getGlobalBounds().contains(mousePos);
        if (isHovered)
            shape.setFillColor(sf::Color::Cyan);
        else
            shape.setFillColor(sf::Color::Blue);
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(shape);
        window.draw(text);
    }
};

// ================= TEXT INPUT FIELD =================
class TextBox
{
public:
    sf::RectangleShape shape;
    sf::Text text;
    string content;
    bool isFocused;
    bool isPassword;
    sf::Font* fontPtr;

    TextBox() : isFocused(false), isPassword(false), fontPtr(nullptr) {}

    TextBox(float x, float y, float width, float height, sf::Font* font, bool password = false)
        : isFocused(false), isPassword(password), fontPtr(font)
    {
        shape.setSize(sf::Vector2f(width, height));
        shape.setPosition(x, y);
        shape.setFillColor(sf::Color::White);
        shape.setOutlineThickness(2.f);
        shape.setOutlineColor(sf::Color::Black);

        if (fontPtr)
        {
            text.setFont(*fontPtr);
            text.setCharacterSize(18);
            text.setFillColor(sf::Color::Black);
            text.setPosition(x + 5, y + 5);
        }
    }

    void handleInput(sf::Event& event)
    {
        if (!isFocused)
            return;

        if (event.type == sf::Event::TextEntered)
        {
            if (event.text.unicode == 8)
            {
                if (content.length() > 0)
                    content.pop_back();
            }
            else if (event.text.unicode == 9)
            {
                // Do nothing
            }
            else if (event.text.unicode >= 32 && event.text.unicode <= 126)
            {
                content += static_cast<char>(event.text.unicode);
            }
            updateDisplay();
        }
    }

    void updateDisplay()
    {
        if (!fontPtr)
            return;

        if (isPassword)
        {
            string display(content.length(), '*');
            text.setString(display);
        }
        else
        {
            text.setString(content);
        }
    }

    void update(sf::Vector2f mousePos, bool mousePressed)
    {
        if (shape.getGlobalBounds().contains(mousePos))
        {
            if (mousePressed)
                isFocused = true;
        }
        else
        {
            if (mousePressed)
                isFocused = false;
        }

        shape.setOutlineColor(isFocused ? sf::Color::Green : sf::Color::Black);
    }

    void draw(sf::RenderWindow& window)
    {
        window.draw(shape);
        window.draw(text);
    }

    void clear()
    {
        content.clear();
        text.setString("");
        isFocused = false;
    }
};

// ================= TIME =================
string getCurrentTime()
{
    time_t now = time(0);
    char* dt = ctime(&now);
    return string(dt);
}

// ================= TRANSACTION =================
class Transaction
{
    string type;
    double amount;
    string dateTime;

public:
    Transaction() : type(""), amount(0), dateTime("") {}

    Transaction(string t, double a, string dt)
    {
        type = t;
        amount = a;
        dateTime = dt;
    }

    void display()
    {
        cout << type << " | " << amount << " | " << dateTime << endl;
    }

    string toString()
    {
        stringstream ss;
        ss << type << " | $" << amount << " | " << dateTime;
        return ss.str();
    }
};

// ================= ACCOUNT =================
class Account
{
protected:
    int accountNumber;
    int pin;
    double balance;
    bool isLocked;
    int attempts;

    Transaction* history;
    int transCount;
    int capacity;

public:
    Account(int accNo, int p, double bal)
    {
        accountNumber = accNo;
        pin = p;
        balance = bal;
        isLocked = false;
        attempts = 0;

        capacity = 5;
        transCount = 0;
        history = new Transaction[capacity];
    }

    virtual ~Account()
    {
        delete[] history;
    }

    int getAccNo() { return accountNumber; }
    int getPin() { return pin; }
    double getBalance() { return balance; }

    virtual int getType() = 0;

    void resize()
    {
        capacity *= 2;
        Transaction* newArr = new Transaction[capacity];

        for (int i = 0; i < transCount; i++)
            newArr[i] = history[i];

        delete[] history;
        history = newArr;
    }

    void addTransaction(string type, double amount)
    {
        if (transCount == capacity)
            resize();

        history[transCount++] = Transaction(type, amount, getCurrentTime());
    }

    bool deposit(double amount)
    {
        if (isLocked) return false;

        balance += amount;
        addTransaction("Deposit", amount);
        return true;
    }

    virtual bool withdraw(double amount)
    {
        if (isLocked) return false;

        if (balance >= amount)
        {
            balance -= amount;
            addTransaction("Withdraw", amount);
            return true;
        }
        return false;
    }

    bool transfer(Account& target, double amount)
    {
        if (withdraw(amount))
        {
            target.deposit(amount);
            addTransaction("Transfer Sent", amount);
            target.addTransaction("Transfer Received", amount);
            return true;
        }
        return false;
    }

    bool verifyPin(int enteredPin)
    {
        if (isLocked) return false;

        if (enteredPin == pin)
        {
            attempts = 0;
            return true;
        }

        attempts++;
        if (attempts >= 3)
            isLocked = true;

        return false;
    }

    void showHistory()
    {
        if (transCount == 0)
        {
            cout << "No transactions yet.\n";
            return;
        }
        cout << "\n--- TRANSACTION HISTORY ---\n";
        for (int i = 0; i < transCount; i++)
            history[i].display();
    }

    Transaction* getHistoryArray()
    {
        return history;
    }

    int getHistoryCount()
    {
        return transCount;
    }
};
// ================= SAVING =================
class SavingAccount : public Account
{
public:
    SavingAccount(int accNo, int p, double bal) : Account(accNo, p, bal) {}

    int getType() override { return 1; }
};

// ================= CURRENT =================
class CurrentAccount : public Account
{
public:
    CurrentAccount(int accNo, int p, double bal) : Account(accNo, p, bal) {}

    int getType() override { return 2; }

    bool withdraw(double amount) override
    {
        if (balance + 1000 >= amount)
        {
            balance -= amount;
            addTransaction("Withdraw(Current)", amount);
            return true;
        }
        return false;
    }
};

// ================= ATM =================
class ATM
{
    Account** accounts;
    int capacity;
    int count;

    string adminUser = "admin";
    string adminPass = "1234";

public:
    ATM()
    {
        capacity = 5;
        count = 0;
        accounts = new Account * [capacity];
    }

    ~ATM()
    {
        for (int i = 0; i < count; i++)
            delete accounts[i];

        delete[] accounts;
    }

    int getCount() { return count; }

    void resize()
    {
        capacity *= 2;
        Account** newArr = new Account * [capacity];

        for (int i = 0; i < count; i++)
            newArr[i] = accounts[i];

        delete[] accounts;
        accounts = newArr;
    }

    void addAccount(Account* acc)
    {
        if (count == capacity)
            resize();

        accounts[count++] = acc;
    }

    bool accountExists(int accNo)
    {
        return findAccount(accNo) != nullptr;
    }

    bool createAccount(int type, int accNo, int pin, double bal)
    {
        if (type != 1 && type != 2)
            return false;

        if (accountExists(accNo))
            return false;

        if (type == 1)
            addAccount(new SavingAccount(accNo, pin, bal));
        else
            addAccount(new CurrentAccount(accNo, pin, bal));

        return true;
    }

    Account* findAccount(int accNo)
    {
        for (int i = 0; i < count; i++)
            if (accounts[i]->getAccNo() == accNo)
                return accounts[i];

        return nullptr;
    }

    Account* userLogin(int accNo, int pin)
    {
        Account* user = findAccount(accNo);

        if (user && user->verifyPin(pin))
            return user;

        return nullptr;
    }

    bool adminLogin(string u, string p)
    {
        return (u == adminUser && p == adminPass);
    }

    void saveToFile()
    {
        ofstream file("accounts.txt");

        if (!file.is_open())
            return;

        for (int i = 0; i < count; i++)
        {
            file << accounts[i]->getType() << " "
                << accounts[i]->getAccNo() << " "
                << accounts[i]->getPin() << " "
                << accounts[i]->getBalance() << endl;
        }

        file.close();
    }

    void loadFromFile()
    {
        ifstream file("accounts.txt");

        if (!file.is_open())
            return;

        int type, accNo, pin;
        double bal;

        while (file >> type >> accNo >> pin >> bal)
        {
            if (type == 1)
                addAccount(new SavingAccount(accNo, pin, bal));
            else
                addAccount(new CurrentAccount(accNo, pin, bal));
        }

        file.close();
    }
};

// ================= SCREEN STATES =================
enum ScreenState
{
    MAIN_MENU,
    ADMIN_LOGIN,
    ADMIN_CREATE_ACCOUNT,
    USER_LOGIN,
    USER_MENU,
    CHECK_BALANCE,
    DEPOSIT,
    WITHDRAW,
    TRANSFER,
    HISTORY,
    FIRST_SETUP
};

// ================= GUI APPLICATION =================
class ATMGui
{
private:
    sf::RenderWindow window;
    sf::Font font;
    ATM atm;
    Account* currentUser;
    ScreenState currentScreen;
    string messageText;
    sf::Clock messageClock;

    // Background image
    sf::Texture backgroundTexture;
    sf::Sprite backgroundSprite;

    Button buttons[6];
    int buttonCount;
    TextBox textBoxes[4];
    int textBoxCount;
    bool mousePressed;

public:
    ATMGui() : window(sf::VideoMode(1920, 1080), "ATM System"), currentUser(nullptr), buttonCount(0), textBoxCount(0), mousePressed(false)
    {
        window.setFramerateLimit(60);

        if (!font.loadFromFile("arial.ttf"))
        {
            cerr << "Error loading font! Make sure arial.ttf is in the project directory.\n";
        }

        // Load background image
        if (!backgroundTexture.loadFromFile("back.png"))
        {
            cerr << "Error loading background! Make sure back.png is in the project directory.\n";
        }

        backgroundSprite.setTexture(backgroundTexture);
        // Scale background to fit window
        float scaleX = window.getSize().x / backgroundTexture.getSize().x;
        float scaleY = window.getSize().y / backgroundTexture.getSize().y;
        backgroundSprite.setScale(scaleX, scaleY);

        atm.loadFromFile();

        if (atm.getCount() == 0)
            switchScreen(FIRST_SETUP);
        else
            switchScreen(MAIN_MENU);
    }

    void switchScreen(ScreenState newState)
    {
        currentScreen = newState;
        buttonCount = 0;
        textBoxCount = 0;
        messageText = "";

        switch (currentScreen)
        {
        case MAIN_MENU:
            buttons[0] = Button(800, 300, 300, 80, "Admin Login", font);
            buttons[1] = Button(800, 420, 300, 80, "User Login", font);
            buttons[2] = Button(800, 540, 300, 80, "Exit", font);
            buttonCount = 3;
            break;

        case ADMIN_LOGIN:
            textBoxes[0] = TextBox(600, 300, 600, 50, &font);
            textBoxes[1] = TextBox(600, 420, 600, 50, &font, true);
            textBoxCount = 2;
            buttons[0] = Button(600, 550, 250, 70, "Login", font);
            buttons[1] = Button(950, 550, 250, 70, "Back", font);
            buttonCount = 2;
            break;

        case USER_LOGIN:
            textBoxes[0] = TextBox(600, 300, 600, 50, &font);
            textBoxes[1] = TextBox(600, 420, 600, 50, &font, true);
            textBoxCount = 2;
            buttons[0] = Button(600, 550, 250, 70, "Login", font);
            buttons[1] = Button(950, 550, 250, 70, "Back", font);
            buttonCount = 2;
            break;

        case USER_MENU:
            buttons[0] = Button(300, 250, 250, 80, "Check Balance", font);
            buttons[1] = Button(1350, 250, 250, 80, "Deposit", font);
            buttons[2] = Button(300, 380, 250, 80, "Withdraw", font);
            buttons[3] = Button(1350, 380, 250, 80, "Transfer", font);
            buttons[4] = Button(300, 510, 250, 80, "History", font);
            buttons[5] = Button(1350, 510, 250, 80, "Logout", font);
            buttonCount = 6;
            break;

        case CHECK_BALANCE:
            buttons[0] = Button(800, 800, 250, 70, "Back", font);
            buttonCount = 1;
            break;

        case DEPOSIT:
            textBoxes[0] = TextBox(600, 350, 600, 50, &font);
            textBoxCount = 1;
            buttons[0] = Button(600, 550, 250, 70, "Deposit", font);
            buttons[1] = Button(950, 550, 250, 70, "Back", font);
            buttonCount = 2;
            break;

        case WITHDRAW:
            textBoxes[0] = TextBox(600, 350, 600, 50, &font);
            textBoxCount = 1;
            buttons[0] = Button(600, 550, 250, 70, "Withdraw", font);
            buttons[1] = Button(950, 550, 250, 70, "Back", font);
            buttonCount = 2;
            break;

        case TRANSFER:
            textBoxes[0] = TextBox(600, 300, 600, 50, &font);
            textBoxes[1] = TextBox(600, 420, 600, 50, &font);
            textBoxCount = 2;
            buttons[0] = Button(600, 600, 250, 70, "Transfer", font);
            buttons[1] = Button(950, 600, 250, 70, "Back", font);
            buttonCount = 2;
            break;

        case HISTORY:
            buttons[0] = Button(800, 950, 250, 70, "Back", font);
            buttonCount = 1;
            break;

        case FIRST_SETUP:
        case ADMIN_CREATE_ACCOUNT:
            textBoxes[0] = TextBox(600, 200, 600, 50, &font);
            textBoxes[1] = TextBox(600, 320, 600, 50, &font);
            textBoxes[2] = TextBox(600, 440, 600, 50, &font, true);
            textBoxes[3] = TextBox(600, 560, 600, 50, &font);
            textBoxCount = 4;
            buttons[0] = Button(600, 700, 250, 70, "Create", font);
            if (currentScreen == ADMIN_CREATE_ACCOUNT)
            {
                buttons[1] = Button(950, 700, 250, 70, "Back", font);
                buttonCount = 2;
            }
            else
            {
                buttonCount = 1;
            }
            break;
        }
    }