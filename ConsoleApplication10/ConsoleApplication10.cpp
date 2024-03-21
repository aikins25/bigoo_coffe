#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <limits>
#include <chrono>
#include <ctime>
#include <thread>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "mysql.h"

using namespace sql;
using namespace std;

void clearScreen() {
    cout << string(50, '\n');
}

bool authenticateAdmin(const string& adminPasscode) {
    string enteredPasscode;
        MYSQL* conn;
        MYSQL_RES* res;
        MYSQL_ROW row;
        const char* server = "localhost";
        const char* user = "root";
        const char* password = "";
        const char* database = "bigoo_cofee";

        conn = mysql_init(NULL);
        if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
            cerr << "Error: " << mysql_error(conn) << endl;
            return false;
        }

        // Execute query to retrieve admin passcode
        if (mysql_query(conn, "SELECT admin_passcode FROM admin_table")) {
            cerr << "Query Error: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return false;
        }

        res = mysql_store_result(conn);

        if (res == NULL) {
            cerr << "Error: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return false;
        }

        // Fetch the admin passcode from the result set
        row = mysql_fetch_row(res);
        if (row == NULL) {
            cerr << "No admin passcode found" << endl;
            mysql_free_result(res);
            mysql_close(conn);
            return false;
        }

        // Compare entered passcode with retrieved admin passcode
        bool authenticated = (enteredPasscode == row[0]);

        // Free result set and close MySQL connection
        mysql_free_result(res);
        mysql_close(conn);

        return authenticated;
}

void changeAdminPasscode(string& adminPasscode) {
    MYSQL* conn;
    const char* server = "localhost";
    const char* user = "root";
    const char* password = "";
    const char* database = "bigoo_cofee";

    conn = mysql_init(NULL);
    if (!conn) {
        cerr << "Error initializing MySQL: " << mysql_error(conn) << endl;
        return;
    }

    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        cerr << "Error connecting to MySQL: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // Prompt for current admin passcode
    cout << "Enter the current admin passcode: ";
    string currentPasscode;
    cin >> currentPasscode;

    // Check if current passcode matches
    if (currentPasscode == adminPasscode) {
        // Prompt for new admin passcode
        cout << "Enter a new admin passcode: ";
        cin >> adminPasscode;

        // Update admin passcode in the database
        string updateQuery = "UPDATE admin_table SET admin_passcode = '" + adminPasscode + "'";
        if (mysql_query(conn, updateQuery.c_str())) {
            cerr << "Query Error: " << mysql_error(conn) << endl;
            mysql_close(conn);
            return;
        }

        cout << "Passcode changed successfully!" << endl;
    }
    else {
        cout << "Invalid current passcode. Passcode change failed." << endl;
    }

    // Close MySQL connection
    mysql_close(conn);
}

void displayLoadingBar(int duration, int totalBars) {
    int progress = 0;
    for (int i = 0; i < totalBars; ++i) {
        progress = (i * 100) / totalBars;
        cout << "[";
        for (int j = 0; j < i; ++j) {
            cout << "=";
        }
        for (int j = i; j < totalBars - 1; ++j) {
            cout << " ";
        }
        cout << "] " << progress << "%" << endl;
        cout.flush();
        this_thread::sleep_for(chrono::milliseconds(duration / totalBars));
    }
}

void printReceipt(string drinkName, double drinkCost, double change, double tipAmount) {
    cout << "==================================== RECEIPT ====================================" << endl;
    cout << "ITEM PURCHASED: " << drinkName << endl;
    cout << "PRICE: C" << fixed << setprecision(2) << drinkCost << endl;
    cout << "TIP: C" << fixed << setprecision(2) << tipAmount << endl;
    cout << "TOTAL PAID: C" << fixed << setprecision(2) << (drinkCost + tipAmount) << endl;
    cout << "CHANGE: C" << fixed << setprecision(2) << change << endl;
    cout << "=================================================================================" << endl;
}

void saveResources(int water, int milk, int coffee, double money) {
    // MySQL connection parameters
    MYSQL* conn;
    const char* server = "localhost";
    const char* user = "root";
    const char* password = "";
    const char* database = "bigoo_cofee"; // Replace with your database name

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        cerr << "Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // Construct SQL query to insert or update resource data
    string query = "INSERT INTO resources (water, milk, coffee, money) VALUES (";
    query += to_string(water) + ", " + to_string(milk) + ", " + to_string(coffee) + ", " + to_string(money) + ") ";
    query += "ON DUPLICATE KEY UPDATE water = VALUES(water), milk = VALUES(milk), coffee = VALUES(coffee), money = VALUES(money)";

    // Execute SQL query
    if (mysql_query(conn, query.c_str())) {
        cerr << "Query Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    cout << "Resources saved to MySQL database successfully!" << endl;

    // Close MySQL connection
    mysql_close(conn);
}

void loadResources(int& water, int& milk, int& coffee, double& money) {
    // MySQL connection parameters
    MYSQL* conn;
    MYSQL_RES* res;
    MYSQL_ROW row;
    const char* server = "localhost";
    const char* user = "root";
    const char* password = "";
    const char* database = "bigoo_cofee"; // Replace with your database name

    conn = mysql_init(NULL);
    if (!mysql_real_connect(conn, server, user, password, database, 0, NULL, 0)) {
        cerr << "Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // Execute SQL query to retrieve resource data
    if (mysql_query(conn, "SELECT water, milk, coffee, money FROM resources")) {
        cerr << "Query Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // Get the result set
    res = mysql_store_result(conn);
    if (res == NULL) {
        cerr << "Result Error: " << mysql_error(conn) << endl;
        mysql_close(conn);
        return;
    }

    // Fetch the first row from the result set
    row = mysql_fetch_row(res);
    if (row == NULL) {
        cerr << "No rows found in the 'resources' table." << endl;
        mysql_free_result(res);
        mysql_close(conn);
        return;
    }

    // Extract resource data from the row
    water = atoi(row[0]);
    milk = atoi(row[1]);
    coffee = atoi(row[2]);
    money = atof(row[3]);

    cout << "Resources loaded from MySQL database successfully!" << endl;

    // Free the result set and close MySQL connection
    mysql_free_result(res);
    mysql_close(conn);
}
void saveToHistory(const string& event, int milkUsed, int milkLeft, int coffeeUsed, int coffeeLeft, int waterUsed,
    int waterLeft, double totalPrice, double tipAmount, double totalPaid, double change) {
    ofstream outFile("C:\\Users\\buabe\\Desktop\\usb\\Emma-Coffe_Machine\\history.txt", ios::app);
    if (outFile.is_open()) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        tm localTime;
        localtime_s(&localTime, &now);

        outFile << event << " at " << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n\n";
        outFile << "==================================== RECEIPT ====================================\n";
        outFile << "ITEM PURCHASED: Multiple Flavors\n";
        outFile << "PRICE: C" << fixed << setprecision(2) << totalPrice << "\n";
        outFile << "TIP: C" << fixed << setprecision(2) << tipAmount << "\n";
        outFile << "TOTAL PAID: C" << fixed << setprecision(2) << totalPaid << "\n";
        outFile << "CHANGE: C" << fixed << setprecision(2) << change << "\n";
        outFile << "=================================================================================\n";
        outFile << "milk used: " << milkUsed << "ml\n";
        outFile << "milk left: " << milkLeft << "ml\n";
        outFile << "coffee used: " << coffeeUsed << "g\n";
        outFile << "coffee left: " << coffeeLeft << "g\n";
        outFile << "water used: " << waterUsed << "ml\n";
        outFile << "water left: " << waterLeft << "ml\n";
        outFile << "\n\n";

        outFile.close();
    }
    else {
        cout << "Error saving history to file." << endl;
    }
}

void saveRestock(const string& resource, int addedAmount, int totalAvailable) {
    ofstream outFile("C:\\Users\\buabe\\Desktop\\usb\\Emma-Coffe_Machine\\history.txt", ios::app);
    if (outFile.is_open()) {
        auto now = chrono::system_clock::to_time_t(chrono::system_clock::now());
        tm localTime;
        localtime_s(&localTime, &now);

        outFile << "Resource Restocked: " << resource << " at " << put_time(&localTime, "%Y-%m-%d %H:%M:%S") << "\n\n";
        outFile << resource << " Added: " << addedAmount << "\n";
        outFile << "Current " << resource << " Available: " << totalAvailable << "\n\n";
        outFile.close();
    }
    else {
        cout << "Error saving restock to file." << endl;
    }
}

int main() {
    int water, milk, coffee;
    double money;
    string adminPasscode = "admin123";

    loadResources(water, milk, coffee, money);

    const double ESPRESSO_COST = 18.00;
    const double LATTE_COST = 30.00;
    const double CAPPUCCINO_COST = 36.00;

    while (true) {
        cout << "===========================================================================================" << endl;
        cout << "                                   WELCOME TO BIGOO COFFEE STATION                           " << endl;
        cout << "===========================================================================================" << endl;
        cout << "                                   WHAT DO YOU WANT TO DO?:                                " << endl;
        cout << "                                   1. Buy a drink                                        " << endl;
        cout << "                                   2. Admin                                             " << endl;
        cout << "                                   3. Turn off the machine                                " << endl;
        cout << "                                   Enter your choice:                                     ";

        int choice;
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "===========================================================================================" << endl;
            cout << "                                   AVAILABLE COFFEE FLAVORS                              " << endl;
            cout << "===========================================================================================" << endl;
            cout << "1. Espresso (C18.00)                                   " << endl;
            cout << "2. Latte (C30.00)                                   " << endl;
            cout << "3. Cappuccino (C36.00)                                   " << endl;
            int numFlavors;
            do {
                cout << "\nHow many different Coffee Flavors would you like to purchase?: ";
                cin >> numFlavors;

                if (cin.fail() || numFlavors < 1 || numFlavors > 3) {
                    cin.clear();
                    cin.ignore(numeric_limits<streamsize>::max(), '\n');
                    cout << "Invalid input. Please enter a number between 1 and 3." << endl;
                }
                else {
                    break;
                }
            } while (true);

            double totalCost = 0.0;
            int totalWater = 0, totalMilk = 0, totalCoffee = 0;

            for (int i = 0; i < numFlavors; ++i) {
                cout << "\nEnter flavor " << i + 1 << " choice: ";
                int drinkChoice;
                cin >> drinkChoice;

                double drinkCost;
                string drinkName;
                int cups;

                switch (drinkChoice) {
                case 1:
                    drinkName = "Espresso";
                    drinkCost = ESPRESSO_COST;
                    break;
                case 2:
                    drinkName = "Latte";
                    drinkCost = LATTE_COST;
                    break;
                case 3:
                    drinkName = "Cappuccino";
                    drinkCost = CAPPUCCINO_COST;
                    break;
                default:
                    cout << "Invalid drink choice." << endl;
                    continue;
                }

                cout << "Drink name is " << drinkName << " and the unit price is C" << fixed << setprecision(2)
                    << drinkCost << "." << endl;
                cout << "How many cups of " << drinkName << " do you wish to have?: ";
                cin >> cups;

                totalCost += drinkCost * cups;
                totalWater += (drinkChoice == 1 ? 50 * cups : (drinkChoice == 2 ? 200 * cups : 250 * cups));
                totalMilk += (drinkChoice == 2 ? 150 * cups : 0);
                totalCoffee += (drinkChoice == 1 ? 18 * cups : 24 * cups);
            }

            cout << "\nTotal price for all selected flavors is C" << fixed << setprecision(2) << totalCost
                << endl;

            cout << "\nPlease insert Money: C";
            double coinsInserted;
            cin >> coinsInserted;

            if (coinsInserted < totalCost) {
                cout << "Insufficient payment. Please insert enough money." << endl;
                continue;
            }

            double change = coinsInserted - totalCost;
            double tipAmount = 0;

            cout << "Would you like to leave a tip? (1. Yes / 2. No): ";
            int tipChoice;
            cin >> tipChoice;
            if (tipChoice == 1) {
                cout << "Enter tip amount: C";
                cin >> tipAmount;
                if (tipAmount > change) {
                    cout << "Invalid tip amount. Tip cannot exceed the change." << endl;
                    continue;
                }
                change -= tipAmount;
            }

            printReceipt("Multiple Flavors", totalCost, change, tipAmount);

            money += (totalCost + tipAmount);

            int milkUsed = totalMilk;
            int coffeeUsed = totalCoffee;
            int waterUsed = totalWater;

            water -= totalWater;
            milk -= totalMilk;
            coffee -= totalCoffee;

            if (water < 0 || milk < 0 || coffee < 0 || money < 0) {
                cout << "Error: Insufficient resources " << endl;
                loadResources(water, milk, coffee, money);
                continue;
            }

            saveResources(water, milk, coffee, money);

            cout << "Please wait, customer's coffee is being prepared" << endl;
            displayLoadingBar(50, 20);

            this_thread::sleep_for(chrono::seconds(5));
            clearScreen();
            cout << "Customer's coffee is ready" << endl;

            saveToHistory("Sale of multiple flavors", milkUsed, milk, coffeeUsed, coffee, waterUsed, water,
                totalCost, tipAmount, (totalCost + tipAmount), change);

            break;
        }
        case 2:
            if (authenticateAdmin(adminPasscode)) {
                cout << "===========================================================================================" << endl;
                cout << "                                   ADMINISTRATION MENU                                   " << endl;
                cout << "===========================================================================================" << endl;
                cout << "                                   1. Check Report                                       " << endl;
                cout << "                                   2. Restock Resources                                  " << endl;
                cout << "                                   3. Change Passcode                                   " << endl;
                cout << "                                   4. Back to Main Menu                                  " << endl;
                cout << "                                   Enter your choice:                                     ";

                int adminChoice;
                cin >> adminChoice;

                switch (adminChoice) {
                case 1:
                    clearScreen();
                    cout << "Water: " << water << "ml\n";
                    cout << "Milk: " << milk << "ml\n";
                    cout << "Coffee: " << coffee << "g\n";
                    cout << "Money: C" << fixed << setprecision(2) << money << endl;
                    break;
                case 2: {
                    clearScreen();
                    cout << "Enter the amount to restock for each resource:" << endl;

                    cout << "Restock Water (ml): ";
                    int restockWater;
                    while (true) {
                        cin >> restockWater;

                        if (cin.fail() || restockWater < 0) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a valid amount." << endl;
                        }
                        else {
                            break;
                        }
                    }

                    cout << "Restock Milk (ml): ";
                    int restockMilk;
                    while (true) {
                        cin >> restockMilk;

                        if (cin.fail() || restockMilk < 0) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a valid amount." << endl;
                        }
                        else {
                            break;
                        }
                    }

                    cout << "Restock Coffee (g): ";
                    int restockCoffee;
                    while (true) {
                        cin >> restockCoffee;

                        if (cin.fail() || restockCoffee < 0) {
                            cin.clear();
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            cout << "Invalid input. Please enter a valid amount." << endl;
                        }
                        else {
                            break;
                        }
                    }

                    water += restockWater;
                    milk += restockMilk;
                    coffee += restockCoffee;

                    saveResources(water, milk, coffee, money);

                    cout << "Resources restocked successfully!" << endl;

                    saveRestock("Milk", restockMilk, milk);
                    saveRestock("Coffee", restockCoffee, coffee);
                    saveRestock("Water", restockWater, water);

                    break;
                }
                case 3:
                    changeAdminPasscode(adminPasscode);
                    break;
                case 4:
                    break;
                default:
                    cout << "Invalid choice. Please try again." << endl;
                }
            }
            else {
                cout << "Authentication failed. Access denied." << endl;
            }
            break;
        case 3:
            cout << "Turning off the Coffee Machine. Goodbye!" << endl;
            return 0;
        default:
            cout << "Invalid choice. Please try again." << endl;
        }
    }

    return 0;
}