-- Таблица пользователей с аутентификацией
CREATE TABLE IF NOT EXISTS users (
    id SERIAL PRIMARY KEY,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(255) UNIQUE NOT NULL,
    password_hash VARCHAR(255) NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Таблица валют
CREATE TABLE IF NOT EXISTS currencies (
    id SERIAL PRIMARY KEY,
    code VARCHAR(3) UNIQUE NOT NULL,
    name VARCHAR(50) NOT NULL,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Таблица счетов
CREATE TABLE IF NOT EXISTS accounts (
    id SERIAL PRIMARY KEY,
    user_id INTEGER NOT NULL REFERENCES users(id) ON DELETE CASCADE,
    currency_id INTEGER NOT NULL REFERENCES currencies(id),
    balance DECIMAL(15, 2) DEFAULT 0.00 CHECK (balance >= 0),
    created_at TIMESTAMPTZ DEFAULT NOW(),
    UNIQUE (user_id, currency_id),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Тип ENUM для статусов переводов
DO $$ BEGIN
    CREATE TYPE transfer_status AS ENUM ('pending', 'completed', 'failed');
EXCEPTION
    WHEN duplicate_object THEN null;
END $$;

-- Таблица переводов с учетом статуса и возможной ошибки
CREATE TABLE IF NOT EXISTS transfers (
    id SERIAL PRIMARY KEY,
    from_account INTEGER NOT NULL REFERENCES accounts(id),
    to_account INTEGER NOT NULL REFERENCES accounts(id),
    amount DECIMAL(15, 2) NOT NULL CHECK (amount > 0),
    status transfer_status NOT NULL DEFAULT 'pending',
    error_message TEXT,
    created_at TIMESTAMPTZ DEFAULT NOW(),
    updated_at TIMESTAMPTZ DEFAULT NOW()
);

-- Функция для очистки error_message и обновления временных меток
CREATE OR REPLACE FUNCTION transfer_audit()
RETURNS TRIGGER AS $$
BEGIN
    -- Обновление updated_at при любом изменении
    NEW.updated_at = NOW();
    
    -- Очистка ошибки при успешном статусе
    IF NEW.status = 'completed' THEN
        NEW.error_message := NULL;
    END IF;
    
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Триггеры
CREATE TRIGGER trg_transfer_audit
BEFORE INSERT OR UPDATE ON transfers
FOR EACH ROW EXECUTE FUNCTION transfer_audit();

-- Индексы
CREATE INDEX IF NOT EXISTS idx_transfers_created ON transfers(created_at);
CREATE INDEX IF NOT EXISTS idx_transfers_status ON transfers(status);
CREATE INDEX IF NOT EXISTS idx_transfers_from ON transfers(from_account);
CREATE INDEX IF NOT EXISTS idx_transfers_to ON transfers(to_account);
CREATE INDEX IF NOT EXISTS idx_transfers_updated ON transfers(updated_at);