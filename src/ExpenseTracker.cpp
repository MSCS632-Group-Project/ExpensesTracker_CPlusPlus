#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <limits>

using namespace std;
/*
 *This program will user to record, view and categorize expenses
 *
 *
 *  ------------+-------------------+-------------+-------------------------------
 *  |   Date     |     Category      |   Amount    |        Description          |
 *  ------------+-------------------+-------------+-------------------------------
 *  | 2025-05-20 | Groceries         |    $45.90   | fruits and veggies          |
 *  | 2025-05-21 | Transportation    |    $10.00   | Bus fare                    |
 *  | 2025-05-22 | Dining Out        |    $25.50   | Lunch with friends          |
 *  ------------+-------------------+-------------+-------------------------------
 */


// Expense class
class Expense {
private:
    tm date;
    double amount;
    string category;
    string description;

public:
    Expense(const tm& dt, double amt, const string& cat, const string& desc = "")
        : date(dt), amount(amt), category(cat), description(desc) {}

    // Getters
    const tm& getDate() const { return date; }
    double getAmount() const { return amount; }
    const string& getCategory() const { return category; }
    const string& getDescription() const { return description; }

    // Date formatting helper
    string getFormattedDate() const {
        char buffer[11];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d", &date);
        return string(buffer);
    }
};

// Expense Tracker class
class ExpenseTracker {
private:
    vector<shared_ptr<Expense>> expenses;

    // Helper function to parse date string
    bool parseDate(const string& dateStr, tm& date) const {
        istringstream iss(dateStr);
        char delimiter;
        int year, month, day;

        if (iss >> year >> delimiter >> month >> delimiter >> day) {
            if (delimiter != '-' || year < 1900 || month < 1 || month > 12 || day < 1 || day > 31) {
                return false;
            }

            date = {};
            date.tm_year = year - 1900;
            date.tm_mon = month - 1;
            date.tm_mday = day;
            date.tm_hour = 0;
            date.tm_min = 0;
            date.tm_sec = 0;
            date.tm_isdst = -1; // Let mktime determine daylight saving time

            // Normalize and validate the date
            time_t temp = mktime(&date);
            if (temp == -1) {
                return false;
            }
            tm* normalized = localtime(&temp);
            if (normalized->tm_year != date.tm_year ||
                normalized->tm_mon != date.tm_mon ||
                normalized->tm_mday != date.tm_mday) {
                return false;
            }

            return true;
        }
        return false;
    }

public:
    bool addExpense(const string& dateStr, double amount, const string& category, const string& description = "") {
        tm date;
        if (!parseDate(dateStr, date)) {
            cerr << "Invalid date format. Please use YYYY-MM-DD.\n";
            return false;
        }

        if (amount <= 0) {
            cerr << "Amount must be positive.\n";
            return false;
        }

        if (category.empty()) {
            cerr << "Category cannot be empty.\n";
            return false;
        }

        expenses.push_back(make_shared<Expense>(date, amount, category, description));
        return true;
    }

    vector<shared_ptr<Expense>> filterExpenses(const string& startDateStr = "",
                                                        const string& endDateStr = "",
                                                        const string& category = "") const {
        vector<shared_ptr<Expense>> filtered;
        tm startDate = {};
        tm endDate = {};
        bool hasStartDate = false;
        bool hasEndDate = false;

        if (!startDateStr.empty()) {
            if (parseDate(startDateStr, startDate)) {
                hasStartDate = true;
            } else {
                cerr << "Invalid start date format. Ignoring.\n";
            }
        }

        if (!endDateStr.empty()) {
            if (parseDate(endDateStr, endDate)) {
                hasEndDate = true;
            } else {
                cerr << "Invalid end date format. Ignoring.\n";
            }
        }

        for (const auto& expense : expenses) {
            bool include = true;

            if (hasStartDate) {
                time_t expTime = mktime(const_cast<tm*>(&expense->getDate()));
                time_t startTime = mktime(&startDate);
                if (expTime < startTime) {
                    include = false;
                }
            }

            if (include && hasEndDate) {
                time_t expTime = mktime(const_cast<tm*>(&expense->getDate()));
                time_t endTime = mktime(&endDate);
                if (expTime > endTime) {
                    include = false;
                }
            }

            if (include && !category.empty()) {
                if (!caseInsensitiveCompare(expense->getCategory(), category)) {
                    include = false;
                }
            }

            if (include) {
                filtered.push_back(expense);
            }
        }

        return filtered;
    }

    struct Summary {
        map<string, double> categories;
        double total;
    };

    Summary getSummary() const {
        Summary summary;
        summary.total = 0.0;

        for (const auto& expense : expenses) {
            const string& category = expense->getCategory();
            double amount = expense->getAmount();

            summary.categories[category] += amount;
            summary.total += amount;
        }

        return summary;
    }

    const vector<shared_ptr<Expense>>& getAllExpenses() const {
        return expenses;
    }

private:
    bool caseInsensitiveCompare(const string& str1, const string& str2) const {
        if (str1.length() != str2.length()) {
            return false;
        }
        return equal(str1.begin(), str1.end(), str2.begin(),
                   [](char a, char b) { return tolower(a) == tolower(b); });
    }
};

// User interface functions
void displayMenu() {
    cout << "\nExpense Tracker Menu:\n";
    cout << "1. Add New Expense\n";
    cout << "2. View All Expenses\n";
    cout << "3. Filter Expenses\n";
    cout << "4. View Summary\n";
    cout << "5. Exit\n";
}

template <typename T>
T getValidInput(const string& prompt) {
    T value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        } else {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid input. Please try again.\n";
        }
    }
}

string getStringInput(const string& prompt, bool allowEmpty = false) {
    string input;
    while (true) {
        cout << prompt;
        getline(cin, input);
        if (!input.empty() || allowEmpty) {
            return input;
        }
        cout << "Input cannot be empty. Please try again.\n";
    }
}

//display all expenses
void displayAllExpenses(const vector<shared_ptr<Expense>>& expenses) {
    cout << "\nAll Expenses:\n";
    cout << left << setw(12) << "Date" << " | "
              << setw(15) << "Category" << " | "
              << right << setw(10) << "Amount" << " | Description\n";
    cout << string(60, '-') << "\n";

    for (const auto& expense : expenses) {
        cout << left << setw(12) << expense->getFormattedDate() << " | "
                  << setw(15) << expense->getCategory() << " | "
                  << right << setw(10) << fixed << setprecision(2)
                  << "$" << expense->getAmount() << " | "
                  << expense->getDescription() << "\n";
    }
}

//it will use date and category to filter if not provided it will be taken as empty filter
void displayFilteredExpenses(const vector<shared_ptr<Expense>>& expenses) {
    cout << "\nFiltered Expenses:\n";
    cout << string(85, '-') << "\n";
    cout << "| " << left << setw(10) << "Date" << " | "
              << setw(20) << "Category" << " | "
              << right << setw(12) << "Amount" << " | "
              << left << setw(31) << "Description" << " |\n";
    cout << string(85, '-') << "\n";

    for (const auto& expense : expenses) {
        cout << "| " << left << setw(10) << expense->getFormattedDate() << " | "
                  << setw(20) << expense->getCategory() << " | "
                  << right << setw(7) << fixed << setprecision(2)
                  << "$" << expense->getAmount() << " | "
                  << left << setw(30) << expense->getDescription() << " |\n";
    }
    cout << string(85, '-') << "\n";
}

//display summary by grouping and calculating total
void displaySummary(const ExpenseTracker::Summary& summary) {
    cout << "\nExpense Summary:\n";
    cout << string(37, '-') << "\n";
    cout << "| " << left << setw(20) << "Category" << " | "
              << right << setw(10) << "Amount" << " |\n";
    cout << string(37, '-') << "\n";

    for (const auto& [category, amount] : summary.categories) {
        cout << "| " << left << setw(20) << category << " | "
                  << right << setw(5) << fixed << setprecision(2)
                  << "$" << amount << " |\n";
    }
    cout << string(37, '-') << "\n";
    cout << "| " << left << setw(20) << "TOTAL" << " | "
              << right << setw(5) << "$" << summary.total << " |\n";
    cout << string(37, '-') << "\n";
}


//main method to execute the program
int main() {
    ExpenseTracker tracker;

    while (true) {
        displayMenu();
        int choice = getValidInput<int>("Enter your choice (1-5): ");

        if (choice == 1) {
            cout << "\nAdd New Expense:\n";
            cout << "(Enter 'q' at any time to cancel)\n";

            string dateStr = getStringInput("Enter date (YYYY-MM-DD): ");
            if (dateStr == "q") {
                cout << "Expense entry cancelled.\n";
                continue;
            }

            double amount = getValidInput<double>("Enter amount: ");
            if (amount <= 0) {
                cout << "Amount must be positive. Try again.\n";
                continue;
            }

            string category = getStringInput("Enter category: ");
            if (category == "q") {
                cout << "Expense entry cancelled.\n";
                continue;
            }

            string description = getStringInput("Enter description (optional): ", true);

            if (tracker.addExpense(dateStr, amount, category, description)) {
                cout << "Expense added successfully!\n";
            }
        }
        else if (choice == 2) {
            displayAllExpenses(tracker.getAllExpenses());
        }
        else if (choice == 3) {
            cout << "\nFilter Expenses:\n";
            string startDate = getStringInput("Enter start date (YYYY-MM-DD, leave empty for no filter): ", true);
            string endDate = getStringInput("Enter end date (YYYY-MM-DD, leave empty for no filter): ", true);
            string category = getStringInput("Enter category to filter by (leave empty for no filter): ", true);

            auto filtered = tracker.filterExpenses(startDate, endDate, category);
            displayFilteredExpenses(filtered);
        }
        else if (choice == 4) {
            auto summary = tracker.getSummary();
            displaySummary(summary);
        }
        else if (choice == 5) {
            cout << "Exiting Expense Tracker. Goodbye!\n";
            break;
        }
        else {
            cout << "Invalid choice. Please enter a number between 1 and 5.\n";
        }
    }

    return 0;
}