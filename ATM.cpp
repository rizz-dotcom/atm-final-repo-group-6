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