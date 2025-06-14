#include "connect_redis.h"

#include <chrono>
#include <stdexcept>

/**
 * @brief Устанавливает соединение с сервером Redis.
 *
 * Создает параметры подключения на основе предоставленной конфигурации и
 * пытается установить соединение с Redis.
 *
 * @param config Объект ConfigRedis, содержащий параметры подключения к Redis.
 * @return Активное соединение с Redis (`sw::redis::Redis`).
 * @throws std::runtime_error В случае ошибок подключения к Redis или системных
 * ошибок, или если индекс базы данных Redis находится вне допустимого диапазона
 * (0-15).
 */
sw::redis::Redis connect_to_redis(const ConfigRedis& config) {
  try {
    sw::redis::ConnectionOptions opts;
    opts.host = config.host;
    opts.port = config.port;
    opts.password = config.password;
    opts.db = config.db;

    opts.socket_timeout = std::chrono::milliseconds(2000);
    opts.connect_timeout = std::chrono::milliseconds(2000);

    if (opts.db < 0 || opts.db > 15) {
      throw std::runtime_error("Invalid Redis DB index: " +
                               std::to_string(opts.db));
    }

    return sw::redis::Redis(opts);
  } catch (const sw::redis::Error& e) {
    throw std::runtime_error("Redis error: " + std::string(e.what()));
  } catch (const std::exception& e) {
    throw std::runtime_error("System error: " + std::string(e.what()));
  }
}
