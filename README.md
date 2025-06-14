# TimmiPay Backend

Этот проект представляет собой бэкэнд-систему для приложения TimmiPay, включающую сервисы аутентификации пользователей и управления финансовыми операциями.

## Назначение проекта

Основная цель проекта TimmiPay Backend — предоставить надежную и масштабируемую основу для обработки пользовательских аутентификаций и финансовых транзакций. Система разработана с учетом модульности, что позволяет легко расширять функционал и интегрировать новые возможности.

## Функционал

Проект состоит из двух основных сервисов:

### 1. Сервис Аутентификации (`auth_service`)

Отвечает за управление пользовательскими сессиями, включая:

*   **Начало сессии (аутентификация)**: Проверка учетных данных пользователя (email и хеш пароля) и выдача токена сессии.
*   **Обновление сессии**: Продление срока действия существующего токена сессии.

**Хранение сессионных токенов:** Сервис аутентификации использует **Redis** для эффективного хранения и управления сессионными токенами. Токены сохраняются в Redis с заданным сроком жизни, что позволяет быстро проверять их валидность и продлевать сессии пользователей.

### 2. Сервис Управления Финансами (`finance_manager`)

Предоставляет API для выполнения финансовых операций:

*   **Получение баланса**: Отображение текущего баланса пользователя по различным валютам.
*   **Перевод денег**: Осуществление переводов средств между пользователями.
*   **История транзакций**: Получение списка всех транзакций пользователя с возможностью пагинации.

## Установка и Запуск

### Требования

*   **CMake** (минимум 3.10)
*   **C++17** совместимый компилятор (GCC, Clang и т.д.)
*   **Git**
*   **Docker** и **Docker Compose** (для PostgreSQL и Redis)
*   **vcpkg** (для управления C++ зависимостями)

### Шаги по установке

1.  **Клонирование репозитория:**

    ```bash
    git clone https://github.com/your-repo/TimmiPay_Backend.git
    cd TimmiPay_Backend
    ```

2.  **Установка vcpkg (если не установлен):**

    Следуйте инструкциям на официальном сайте vcpkg или выполните:

    ```bash
    git clone https://github.com/Microsoft/vcpkg.git
    ./vcpkg/bootstrap-vcpkg.sh # или bootstrap-vcpkg.bat для Windows
    ./vcpkg/vcpkg integrate install
    ```

3.  **Установка C++ зависимостей через vcpkg:**

    Проект использует следующие библиотеки. `vcpkg` автоматически установит их при первой сборке CMake, но вы также можете установить их вручную:

    ```bash
    ./vcpkg/vcpkg install nlohmann-json:x64-linux@3.12.0
    ./vcpkg/vcpkg install gtest:x64-linux@1.16.0#1
    ./vcpkg/vcpkg install libpqxx:x64-linux@7.10.0
    ./vcpkg/vcpkg install boost-uuid:x64-linux@1.88.0
    ./vcpkg/vcpkg install redis-plus-plus:x64-linux@1.3.14
    ./vcpkg/vcpkg install crow:x64-linux@1.2.1.2
    ./vcpkg/vcpkg install curl:x64-linux@8.14.1
    ```
    (Обратите внимание, что версии пакетов могут отличаться в зависимости от актуального состояния vcpkg. Если возникнут ошибки, попробуйте установить пакеты без указания версии, например: `./vcpkg/vcpkg install nlohmann-json`.)

4.  **Запуск Баз Данных (PostgreSQL и Redis) через Docker Compose:**

    Убедитесь, что Docker запущен.

    ```bash
    docker-compose up -d
    ```

    Это поднимет контейнеры PostgreSQL и Redis. Инициализация базы данных PostgreSQL будет выполнена автоматически с помощью файла `init.sql`.

5.  **Сборка проекта с CMake:**

    Создайте директорию для сборки, перейдите в нее и скомпилируйте проект:

    ```bash
    mkdir build
    cd build
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build .
    ```

    Если вы используете другую директорию для `vcpkg`, убедитесь, что путь к `CMAKE_TOOLCHAIN_FILE` указан правильно.

### Запуск сервисов

После успешной сборки исполняемые файлы будут находиться в директории `build`.

*   **Запуск сервиса аутентификации:**
    ```bash
    ./auth_service
    ```
*   **Запуск сервиса управления финансами:**
    ```bash
    ./finance_manager
    ```
    Каждый сервис по умолчанию будет слушать на своем порту (Crow по умолчанию 18080).

## Тестирование

Проект использует **Google Test** для модульного тестирования. Тесты интегрированы с `ctest`.

Для запуска всех тестов из директории сборки:

```bash
cd build
ctest
```

## Примеры использования API

Ниже приведены примеры использования основных эндпоинтов API с помощью `curl`. Предполагается, что сервисы запущены и доступны на `http://localhost:18080` (стандартный порт Crow).

### 1. Сервис Аутентификации (`auth_service`)

#### 1.1. Начало сессии (Аутентификация)

*   **Эндпоинт:** `/session_start`
*   **Метод:** `POST`
*   **Описание:** Аутентификация пользователя по email и хешу пароля.
*   **Запрос:**
    ```bash
    curl -X POST http://localhost:18080/session_start -H "Content-Type: application/json" -d '{
        "email": "user@example.com",
        "password_hash": "your_hashed_password"
    }'
    ```
*   **Пример успешного ответа:**
    ```json
    {
        "token": "generated_session_token"
    }
    ```
*   **Пример ответа с ошибкой:**
    ```json
    {
        "error": "Invalid credentials"
    }
    ```

#### 1.2. Обновление сессии

*   **Эндпоинт:** `/session_refresh`
*   **Метод:** `POST`
*   **Описание:** Обновление срока действия существующего токена сессии.
*   **Запрос:**
    ```bash
    curl -X POST http://localhost:18080/session_refresh -H "Content-Type: application/json" -d '{
        "token": "existing_session_token"
    }'
    ```
*   **Пример успешного ответа:**
    ```json
    {
        "status": "success",
        "message": "Session token refreshed."
    }
    ```
*   **Пример ответа с ошибкой:**
    ```json
    {
        "status": "error",
        "message": "Invalid or expired token."
    }
    ```

### 2. Сервис Управления Финансами (`finance_manager`)

*   **Примечание:** Для всех запросов к `finance_manager` требуется валидный `session_token`, полученный от `auth_service`.

#### 2.1. Получение баланса пользователя

*   **Эндпоинт:** `/api/v1/balance`
*   **Метод:** `POST`
*   **Описание:** Получение баланса пользователя по всем валютам.
*   **Запрос:**
    ```bash
    curl -X POST http://localhost:18080/api/v1/balance -H "Content-Type: application/json" -d '{
        "session_token": "valid_session_token"
    }'
    ```
*   **Пример успешного ответа:**
    ```json
    [
        {
            "currency": "USD",
            "balance": 1500.75
        },
        {
            "currency": "EUR",
            "balance": 500.00
        }
    ]
    ```
*   **Пример ответа с ошибкой (неверный токен):**
    ```
    Invalid session token
    ```
*   **Пример ответа с ошибкой (внутренняя ошибка):**
    ```
    Internal server error
    ```

#### 2.2. Перевод денег

*   **Эндпоинт:** `/api/v1/transfer`
*   **Метод:** `POST`
*   **Описание:** Перевод средств между пользователями.
*   **Запрос:**
    ```bash
    curl -X POST http://localhost:18080/api/v1/transfer -H "Content-Type: application/json" -d '{
        "session_token": "valid_session_token",
        "to_username": "recipient_username",
        "amount": 100.00,
        "currency": "USD"
    }'
    ```
*   **Пример успешного ответа:**
    ```json
    {
        "transfer_id": "unique_transfer_id"
    }
    ```
*   **Пример ответа с ошибкой (неверный токен):**
    ```
    Invalid session token
    ```
*   **Пример ответа с ошибкой (недостаточно средств/бизнес-логика):**
    ```json
    {
        "error": "Insufficient funds"
    }
    ```
*   **Пример ответа с ошибкой (внутренняя ошибка):**
    ```json
    {
        "error": "Internal server error"
    }
    ```

#### 2.3. История транзакций

*   **Эндпоинт:** `/api/v1/history`
*   **Метод:** `POST`
*   **Описание:** Получение истории транзакций пользователя с опциональной пагинацией.
*   **Запрос:**
    ```bash
    curl -X POST http://localhost:18080/api/v1/history -H "Content-Type: application/json" -d '{
        "session_token": "valid_session_token",
        "page": 1,
        "limit": 5
    }'
    ```
    Параметры `page` и `limit` опциональны. Если не указаны, используются значения по умолчанию (page=1, limit=10).
*   **Пример успешного ответа:**
    ```json
    [
        {
            "transfer_id": "transfer_id_1",
            "amount": 50.00,
            "status": "completed",
            "created_at": "2023-10-27T10:00:00Z"
        },
        {
            "transfer_id": "transfer_id_2",
            "amount": 25.50,
            "status": "pending",
            "created_at": "2023-10-27T09:30:00Z"
        }
    ]
    ```
*   **Пример ответа с ошибкой (неверный токен):**
    ```
    Invalid session token
    ```
*   **Пример ответа с ошибкой (внутренняя ошибка):**
    ```json
    {
        "error": "Internal server error"
    }
    ``` 
