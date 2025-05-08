#ifndef USER_BALANCE_H
#define USER_BALANCE_H

#include <pqxx/pqxx>
#include <unordered_map>
#include <string>

class BalanceStorage {
public:
    BalanceStorage(pqxx::connection& conn);
    std::unordered_map<std::string, double> GetUserBalances(const std::string& user_id);
    
private:
    pqxx::connection& conn_;
};

#endif