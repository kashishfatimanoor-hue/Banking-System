// ============================================================
//  Banking System  (C++)
// ------------------------------------------------------------
//  Classes:
//    Transaction : one record in an account's history
//                  (id, date/time, type, amount, balance after, details)
//    Account     : account number, owner, type, balance and its
//                  list of transactions (deposit / withdraw)
//    Customer    : customer id, name, phone number and the
//                  numbers of the accounts the customer owns
//    Bank        : keeps all customers and accounts and does the
//                  work: create customers, open accounts, deposit,
//                  withdraw, transfer, and show information
//
//  Features:
//    - Create customers and open accounts (Savings or Current)
//    - Deposits, withdrawals and fund transfers between accounts
//    - Every operation is stored as a transaction in the history
//    - View balance, account details and recent transactions
//    - All inputs are validated and clear messages are shown
//
//  Notes:
//    - Money is rounded to 2 decimal places after every operation,
//      so there are no tiny calculation errors.
//    - Data is kept in memory while the program is running.
// ============================================================

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <cctype>
#include <cstdlib>
#include <cmath>
#include <ctime>
using namespace std;

// ---------- Settings ----------
const double MAX_TRANSACTION_AMOUNT = 10000000.00;  // largest amount for one operation
const size_t MAX_ACCOUNTS_PER_CUSTOMER = 5;
const int RECENT_ON_DETAILS_SCREEN = 5;             // transactions shown in "account details"

// ============================================================
//  PART 1: Small helper functions
// ============================================================

// Rounds a money value to 2 decimal places.
double roundToCents(double value) {
    return floor(value * 100.0 + 0.5) / 100.0;
}

// Turns a money value into text with exactly 2 decimals, e.g. 1500.00
string formatMoney(double value) {
    ostringstream out;
    out << fixed << setprecision(2) << value;
    return out.str();
}

// Turns a whole number into text.
string intToString(int value) {
    ostringstream out;
    out << value;
    return out.str();
}

// Returns the current date and time as text, e.g. 2026-10-24 14:30:05
string currentTimeText() {
    time_t now = time(NULL);
    struct tm *localTime = localtime(&now);
    if (localTime == NULL)
        return "unknown time";

    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localTime);
    return string(buffer);
}

// Removes spaces at the start and end of a string.
string trim(const string &text) {
    size_t start = 0;
    while (start < text.size() && isspace(static_cast<unsigned char>(text[start]))) start++;
    size_t end = text.size();
    while (end > start && isspace(static_cast<unsigned char>(text[end - 1]))) end--;
    return text.substr(start, end - start);
}

// ============================================================
//  PART 2: The classes
// ============================================================

// ---------- Transaction ----------
class Transaction {
private:
    int id;
    string timestamp;
    string type;          // "Deposit", "Withdrawal", "Transfer Out" or "Transfer In"
    double amount;
    double balanceAfter;  // balance of the account after this transaction
    string details;       // short note, e.g. "To account 1002"

public:
    Transaction(int transactionId, const string &transactionType, double transactionAmount,
                double newBalance, const string &note)
        : id(transactionId),
          timestamp(currentTimeText()),
          type(transactionType),
          amount(transactionAmount),
          balanceAfter(newBalance),
          details(note) {}

    int getId() const { return id; }
    const string &getTimestamp() const { return timestamp; }
    const string &getType() const { return type; }
    double getAmount() const { return amount; }
    double getBalanceAfter() const { return balanceAfter; }
    const string &getDetails() const { return details; }
};

// ---------- Account ----------
class Account {
private:
    int accountNumber;
    int ownerId;              // id of the customer who owns this account
    string accountType;       // "Savings" or "Current"
    double balance;
    vector<Transaction> history;

public:
    Account(int number, int customerId, const string &type)
        : accountNumber(number), ownerId(customerId), accountType(type), balance(0.0) {}

    int getAccountNumber() const { return accountNumber; }
    int getOwnerId() const { return ownerId; }
    const string &getAccountType() const { return accountType; }
    double getBalance() const { return balance; }
    const vector<Transaction> &getHistory() const { return history; }

    // Returns true if the account has enough money for this amount.
    bool hasEnoughFunds(double amount) const {
        return roundToCents(amount) <= balance;
    }

    // Adds money to the account and records the transaction.
    void credit(double amount, const string &type, const string &details, int transactionId) {
        balance = roundToCents(balance + amount);
        history.push_back(Transaction(transactionId, type, roundToCents(amount), balance, details));
    }

    // Takes money from the account and records the transaction.
    // Returns false (and changes nothing) if the funds are not enough.
    bool debit(double amount, const string &type, const string &details, int transactionId) {
        if (!hasEnoughFunds(amount))
            return false;

        balance = roundToCents(balance - amount);
        history.push_back(Transaction(transactionId, type, roundToCents(amount), balance, details));
        return true;
    }
};

// ---------- Customer ----------
class Customer {
private:
    int customerId;
    string name;
    string phone;
    vector<int> accountNumbers;   // accounts owned by this customer

public:
    Customer(int id, const string &customerName, const string &customerPhone)
        : customerId(id), name(customerName), phone(customerPhone) {}

    int getCustomerId() const { return customerId; }
    const string &getName() const { return name; }
    const string &getPhone() const { return phone; }
    const vector<int> &getAccountNumbers() const { return accountNumbers; }
    size_t getAccountCount() const { return accountNumbers.size(); }

    void addAccountNumber(int accountNumber) { accountNumbers.push_back(accountNumber); }
};

// ============================================================
//  PART 3: Showing transactions in a table
// ============================================================

// Shows the most recent transactions of an account (newest first).
void printTransactions(const Account &account, size_t count) {
    const vector<Transaction> &history = account.getHistory();
    size_t total = history.size();

    if (total == 0) {
        cout << "No transactions yet for this account.\n";
        return;
    }

    size_t shown = (count < total) ? count : total;
    cout << "Showing the last " << shown << " of " << total << " transaction(s), newest first:\n";

    cout << left << setw(6) << "ID"
         << setw(21) << "Date & Time"
         << setw(14) << "Type"
         << setw(14) << "Amount"
         << setw(16) << "Balance After"
         << "Details\n";
    cout << "-----------------------------------------------------------------------------------\n";

    for (size_t i = 0; i < shown; i++) {
        const Transaction &t = history[total - 1 - i];
        cout << left << setw(6) << t.getId()
             << setw(21) << t.getTimestamp()
             << setw(14) << t.getType()
             << setw(14) << formatMoney(t.getAmount())
             << setw(16) << formatMoney(t.getBalanceAfter())
             << t.getDetails() << "\n";
    }
}

// ============================================================
//  PART 4: The Bank (does all the work)
// ============================================================

class Bank {
private:
    vector<Customer> customers;
    vector<Account> accounts;
    int nextCustomerId;
    int nextAccountNumber;
    int nextTransactionId;

    // Position of a customer / account in the vectors, or -1 if not found.
    int findCustomerIndex(int customerId) const {
        for (size_t i = 0; i < customers.size(); i++)
            if (customers[i].getCustomerId() == customerId)
                return static_cast<int>(i);
        return -1;
    }

    int findAccountIndex(int accountNumber) const {
        for (size_t i = 0; i < accounts.size(); i++)
            if (accounts[i].getAccountNumber() == accountNumber)
                return static_cast<int>(i);
        return -1;
    }

public:
    Bank() : nextCustomerId(1), nextAccountNumber(1001), nextTransactionId(1) {}

    bool customerExists(int customerId) const { return findCustomerIndex(customerId) >= 0; }
    bool accountExists(int accountNumber) const { return findAccountIndex(accountNumber) >= 0; }

    // ----- Create a customer -----
    void createCustomer(const string &name, const string &phone) {
        for (size_t i = 0; i < customers.size(); i++) {
            if (customers[i].getPhone() == phone) {
                cout << "Error: a customer with this phone number already exists "
                     << "(Customer ID " << customers[i].getCustomerId() << ").\n";
                return;
            }
        }

        Customer customer(nextCustomerId, name, phone);
        customers.push_back(customer);
        cout << "Customer created successfully!\n";
        cout << "  Customer ID : " << nextCustomerId << "\n";
        cout << "  Name        : " << name << "\n";
        nextCustomerId++;
    }

    // ----- Open an account for a customer -----
    void openAccount(int customerId, const string &type, double initialDeposit) {
        int ci = findCustomerIndex(customerId);
        if (ci < 0) {
            cout << "Error: no customer found with ID " << customerId << ".\n";
            return;
        }
        if (customers[ci].getAccountCount() >= MAX_ACCOUNTS_PER_CUSTOMER) {
            cout << "Error: a customer can have at most " << MAX_ACCOUNTS_PER_CUSTOMER << " accounts.\n";
            return;
        }

        Account account(nextAccountNumber, customerId, type);
        if (initialDeposit > 0.0) {
            account.credit(initialDeposit, "Deposit", "Account opening deposit", nextTransactionId);
            nextTransactionId++;
        }

        accounts.push_back(account);
        customers[ci].addAccountNumber(nextAccountNumber);

        cout << "Account opened successfully!\n";
        cout << "  Account number : " << nextAccountNumber << "\n";
        cout << "  Account type   : " << type << "\n";
        cout << "  Owner          : " << customers[ci].getName() << "\n";
        cout << "  Balance        : " << formatMoney(account.getBalance()) << "\n";
        nextAccountNumber++;
    }

    // ----- Deposit -----
    void deposit(int accountNumber, double amount) {
        int ai = findAccountIndex(accountNumber);
        if (ai < 0) {
            cout << "Error: account " << accountNumber << " does not exist.\n";
            return;
        }

        accounts[ai].credit(amount, "Deposit", "Cash deposit", nextTransactionId);
        nextTransactionId++;

        cout << "Deposit successful! Amount: " << formatMoney(amount)
             << " | New balance: " << formatMoney(accounts[ai].getBalance()) << "\n";
    }

    // ----- Withdraw -----
    void withdraw(int accountNumber, double amount) {
        int ai = findAccountIndex(accountNumber);
        if (ai < 0) {
            cout << "Error: account " << accountNumber << " does not exist.\n";
            return;
        }

        if (!accounts[ai].debit(amount, "Withdrawal", "Cash withdrawal", nextTransactionId)) {
            cout << "Error: insufficient funds. Available balance: "
                 << formatMoney(accounts[ai].getBalance()) << "\n";
            return;
        }
        nextTransactionId++;

        cout << "Withdrawal successful! Amount: " << formatMoney(amount)
             << " | New balance: " << formatMoney(accounts[ai].getBalance()) << "\n";
    }

    // ----- Transfer between two accounts -----
    void transfer(int fromNumber, int toNumber, double amount) {
        if (fromNumber == toNumber) {
            cout << "Error: you cannot transfer money to the same account.\n";
            return;
        }

        int fromIndex = findAccountIndex(fromNumber);
        int toIndex = findAccountIndex(toNumber);

        if (fromIndex < 0) {
            cout << "Error: sender account " << fromNumber << " does not exist.\n";
            return;
        }
        if (toIndex < 0) {
            cout << "Error: receiver account " << toNumber << " does not exist.\n";
            return;
        }
        if (!accounts[fromIndex].hasEnoughFunds(amount)) {
            cout << "Error: insufficient funds. Available balance: "
                 << formatMoney(accounts[fromIndex].getBalance()) << "\n";
            return;
        }

        // Both accounts exist and the funds are enough, so the transfer can be done.
        accounts[fromIndex].debit(amount, "Transfer Out", "To account " + intToString(toNumber), nextTransactionId);
        nextTransactionId++;
        accounts[toIndex].credit(amount, "Transfer In", "From account " + intToString(fromNumber), nextTransactionId);
        nextTransactionId++;

        cout << "Transfer successful! Amount: " << formatMoney(amount) << "\n";
        cout << "  Account " << fromNumber << " new balance: " << formatMoney(accounts[fromIndex].getBalance()) << "\n";
        cout << "  Account " << toNumber << " new balance: " << formatMoney(accounts[toIndex].getBalance()) << "\n";
    }

    // ----- Show account information (balance + latest transactions) -----
    void showAccountDetails(int accountNumber) const {
        int ai = findAccountIndex(accountNumber);
        if (ai < 0) {
            cout << "Error: account " << accountNumber << " does not exist.\n";
            return;
        }

        const Account &account = accounts[ai];
        int ci = findCustomerIndex(account.getOwnerId());

        cout << "\n----- ACCOUNT DETAILS -----\n";
        cout << "Account number : " << account.getAccountNumber() << "\n";
        cout << "Account type   : " << account.getAccountType() << "\n";
        if (ci >= 0)
            cout << "Owner          : " << customers[ci].getName()
                 << " (Customer ID " << customers[ci].getCustomerId() << ")\n";
        cout << "Balance        : " << formatMoney(account.getBalance()) << "\n";
        cout << "Transactions   : " << account.getHistory().size() << "\n\n";
        printTransactions(account, RECENT_ON_DETAILS_SCREEN);
    }

    // ----- Show the most recent transactions of an account -----
    void showRecentTransactions(int accountNumber, int count) const {
        int ai = findAccountIndex(accountNumber);
        if (ai < 0) {
            cout << "Error: account " << accountNumber << " does not exist.\n";
            return;
        }

        cout << "\n----- TRANSACTIONS OF ACCOUNT " << accountNumber << " -----\n";
        printTransactions(accounts[ai], static_cast<size_t>(count));
    }

    // ----- Show a customer and all of his/her accounts -----
    void showCustomerAccounts(int customerId) const {
        int ci = findCustomerIndex(customerId);
        if (ci < 0) {
            cout << "Error: no customer found with ID " << customerId << ".\n";
            return;
        }

        const Customer &customer = customers[ci];
        cout << "\n----- CUSTOMER DETAILS -----\n";
        cout << "Customer ID : " << customer.getCustomerId() << "\n";
        cout << "Name        : " << customer.getName() << "\n";
        cout << "Phone       : " << customer.getPhone() << "\n";

        if (customer.getAccountCount() == 0) {
            cout << "This customer has no accounts yet.\n";
            return;
        }

        cout << "\n" << left << setw(16) << "Account Number"
             << setw(12) << "Type"
             << "Balance\n";
        cout << "------------------------------------\n";

        double totalBalance = 0.0;
        const vector<int> &numbers = customer.getAccountNumbers();
        for (size_t i = 0; i < numbers.size(); i++) {
            int ai = findAccountIndex(numbers[i]);
            if (ai < 0)
                continue;
            cout << left << setw(16) << accounts[ai].getAccountNumber()
                 << setw(12) << accounts[ai].getAccountType()
                 << formatMoney(accounts[ai].getBalance()) << "\n";
            totalBalance = roundToCents(totalBalance + accounts[ai].getBalance());
        }
        cout << "------------------------------------\n";
        cout << left << setw(28) << "Total balance" << formatMoney(totalBalance) << "\n";
    }

    // ----- List all customers -----
    void listCustomers() const {
        if (customers.empty()) {
            cout << "There are no customers yet.\n";
            return;
        }

        cout << "\n" << left << setw(8) << "ID"
             << setw(28) << "Name"
             << setw(18) << "Phone"
             << "Accounts\n";
        cout << "---------------------------------------------------------------\n";
        for (size_t i = 0; i < customers.size(); i++) {
            cout << left << setw(8) << customers[i].getCustomerId()
                 << setw(28) << customers[i].getName()
                 << setw(18) << customers[i].getPhone()
                 << customers[i].getAccountCount() << "\n";
        }
    }
};

// ============================================================
//  PART 5: Reading and validating user input
// ============================================================

// Reads one line from the user. Ends the program if the input is closed.
string readLine(const string &prompt) {
    string line;
    cout << prompt;
    if (!getline(cin, line)) {
        cout << "\nInput ended. Exiting program.\n";
        exit(1);
    }
    return trim(line);
}

// Reads a whole number between minValue and maxValue (asks again until valid).
int readInteger(const string &prompt, int minValue, int maxValue) {
    while (true) {
        string line = readLine(prompt);

        bool onlyDigits = !line.empty() && line.size() <= 9;
        for (size_t i = 0; i < line.size() && onlyDigits; i++)
            if (!isdigit(static_cast<unsigned char>(line[i])))
                onlyDigits = false;

        if (onlyDigits) {
            int value = atoi(line.c_str());
            if (value >= minValue && value <= maxValue)
                return value;
        }
        cout << "  Invalid input. Please enter a whole number from "
             << minValue << " to " << maxValue << ".\n";
    }
}

// Reads a money amount with at most 2 decimals, e.g. 250 or 99.50
// If allowZero is false, the amount must be at least 0.01
double readAmount(const string &prompt, bool allowZero) {
    while (true) {
        string line = readLine(prompt);

        // Only digits and at most one dot are allowed.
        bool valid = !line.empty();
        int dots = 0;
        int digitsAfterDot = 0;
        int digitCount = 0;
        for (size_t i = 0; i < line.size() && valid; i++) {
            if (line[i] == '.') {
                dots++;
                if (dots > 1) valid = false;
            } else if (isdigit(static_cast<unsigned char>(line[i]))) {
                digitCount++;
                if (dots == 1) digitsAfterDot++;
            } else {
                valid = false;
            }
        }
        if (digitCount == 0 || digitsAfterDot > 2)
            valid = false;

        if (valid) {
            double value = roundToCents(strtod(line.c_str(), NULL));
            double minimum = allowZero ? 0.0 : 0.01;
            if (value >= minimum && value <= MAX_TRANSACTION_AMOUNT)
                return value;
        }

        cout << "  Invalid amount. Enter a number with at most 2 decimals ("
             << (allowZero ? "0" : "0.01") << " to " << formatMoney(MAX_TRANSACTION_AMOUNT) << ").\n";
    }
}

// Reads a valid customer name.
string readName() {
    while (true) {
        string name = readLine("Enter customer name: ");

        bool hasLetter = false;
        bool validChars = true;
        for (size_t i = 0; i < name.size(); i++) {
            unsigned char ch = static_cast<unsigned char>(name[i]);
            if (isalpha(ch))
                hasLetter = true;
            else if (ch != ' ' && ch != '.' && ch != '-' && ch != '\'')
                validChars = false;
        }

        if (name.size() < 2 || name.size() > 40)
            cout << "  Name must be 2 to 40 characters long.\n";
        else if (!validChars || !hasLetter)
            cout << "  Name can only contain letters, spaces, dots, hyphens and apostrophes.\n";
        else
            return name;
    }
}

// Reads a valid phone number (digits only, 10 to 15 digits).
string readPhone() {
    while (true) {
        string phone = readLine("Enter phone number (digits only): ");

        bool onlyDigits = !phone.empty();
        for (size_t i = 0; i < phone.size(); i++)
            if (!isdigit(static_cast<unsigned char>(phone[i])))
                onlyDigits = false;

        if (onlyDigits && phone.size() >= 10 && phone.size() <= 15)
            return phone;
        cout << "  Phone number must contain 10 to 15 digits and nothing else.\n";
    }
}

// Reads an account number and checks that the account exists.
// Returns true and fills 'number' if it exists.
bool readExistingAccount(const Bank &bank, const string &prompt, int &number) {
    number = readInteger(prompt, 1, 999999999);
    if (!bank.accountExists(number)) {
        cout << "Error: account " << number << " does not exist.\n";
        return false;
    }
    return true;
}

// ============================================================
//  PART 6: Menu actions
// ============================================================

void actionCreateCustomer(Bank &bank) {
    cout << "\n--- CREATE CUSTOMER ---\n";
    string name = readName();
    string phone = readPhone();
    bank.createCustomer(name, phone);
}

void actionOpenAccount(Bank &bank) {
    cout << "\n--- OPEN ACCOUNT ---\n";
    int customerId = readInteger("Enter customer ID: ", 1, 999999999);
    if (!bank.customerExists(customerId)) {
        cout << "Error: no customer found with ID " << customerId << ".\n";
        return;
    }

    cout << "Account type:  1. Savings   2. Current\n";
    int typeChoice = readInteger("Choose account type (1-2): ", 1, 2);
    string type = (typeChoice == 1) ? "Savings" : "Current";

    double initialDeposit = readAmount("Enter opening deposit (0 for none): ", true);
    bank.openAccount(customerId, type, initialDeposit);
}

void actionDeposit(Bank &bank) {
    cout << "\n--- DEPOSIT ---\n";
    int accountNumber;
    if (!readExistingAccount(bank, "Enter account number: ", accountNumber))
        return;
    double amount = readAmount("Enter amount to deposit: ", false);
    bank.deposit(accountNumber, amount);
}

void actionWithdraw(Bank &bank) {
    cout << "\n--- WITHDRAW ---\n";
    int accountNumber;
    if (!readExistingAccount(bank, "Enter account number: ", accountNumber))
        return;
    double amount = readAmount("Enter amount to withdraw: ", false);
    bank.withdraw(accountNumber, amount);
}

void actionTransfer(Bank &bank) {
    cout << "\n--- TRANSFER FUNDS ---\n";
    int fromNumber, toNumber;
    if (!readExistingAccount(bank, "Enter sender account number: ", fromNumber))
        return;
    if (!readExistingAccount(bank, "Enter receiver account number: ", toNumber))
        return;
    double amount = readAmount("Enter amount to transfer: ", false);
    bank.transfer(fromNumber, toNumber, amount);
}

void actionViewAccount(const Bank &bank) {
    cout << "\n--- VIEW ACCOUNT DETAILS ---\n";
    int accountNumber;
    if (!readExistingAccount(bank, "Enter account number: ", accountNumber))
        return;
    bank.showAccountDetails(accountNumber);
}

void actionViewTransactions(const Bank &bank) {
    cout << "\n--- RECENT TRANSACTIONS ---\n";
    int accountNumber;
    if (!readExistingAccount(bank, "Enter account number: ", accountNumber))
        return;
    int count = readInteger("How many recent transactions to show (1-50)? ", 1, 50);
    bank.showRecentTransactions(accountNumber, count);
}

void actionViewCustomer(const Bank &bank) {
    cout << "\n--- VIEW CUSTOMER ACCOUNTS ---\n";
    int customerId = readInteger("Enter customer ID: ", 1, 999999999);
    bank.showCustomerAccounts(customerId);
}

// ============================================================
//  PART 7: Main menu
// ============================================================

int main() {
    Bank bank;

    cout << "=========================================\n";
    cout << "             BANKING SYSTEM              \n";
    cout << "=========================================\n";

    while (true) {
        cout << "\nMAIN MENU\n";
        cout << "  1. Create new customer\n";
        cout << "  2. Open new account\n";
        cout << "  3. Deposit money\n";
        cout << "  4. Withdraw money\n";
        cout << "  5. Transfer funds\n";
        cout << "  6. View account details (balance and latest transactions)\n";
        cout << "  7. View recent transactions\n";
        cout << "  8. View a customer's accounts\n";
        cout << "  9. List all customers\n";
        cout << "  0. Exit\n";

        int choice = readInteger("Enter your choice (0-9): ", 0, 9);

        switch (choice) {
            case 1: actionCreateCustomer(bank); break;
            case 2: actionOpenAccount(bank); break;
            case 3: actionDeposit(bank); break;
            case 4: actionWithdraw(bank); break;
            case 5: actionTransfer(bank); break;
            case 6: actionViewAccount(bank); break;
            case 7: actionViewTransactions(bank); break;
            case 8: actionViewCustomer(bank); break;
            case 9: bank.listCustomers(); break;
            case 0:
                cout << "Thank you for using the Banking System. Goodbye!\n";
                return 0;
        }
    }
}