#pragma once

#include <pqxx/pqxx>
#include "../config/config.h"

pqxx::connection connect_to_database(const Config& config);