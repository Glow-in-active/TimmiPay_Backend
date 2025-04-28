#include "uuid_generator.h"
#include <gtest/gtest.h>
#include <regex>

TEST(UUIDGeneratorTest, GeneratesValidFormat) {
    UUIDGenerator generator;
    const std::string uuid = generator.generateUUID();
    
    const std::regex uuid_pattern(
        "[a-fA-F0-9]{8}-"
        "[a-fA-F0-9]{4}-"
        "4[a-fA-F0-9]{3}-"
        "[89aAbB][a-fA-F0-9]{3}-"
        "[a-fA-F0-9]{12}"
    );
    
    EXPECT_TRUE(std::regex_match(uuid, uuid_pattern)) 
        << "Invalid UUID format: " << uuid;
}

TEST(UUIDGeneratorTest, GeneratesUniqueValues) {
    UUIDGenerator generator;
    const std::string uuid1 = generator.generateUUID();
    const std::string uuid2 = generator.generateUUID();
    
    EXPECT_NE(uuid1, uuid2) 
        << "Generated identical UUIDs: " << uuid1 << " and " << uuid2;
}

TEST(UUIDGeneratorTest, NoExceptionsThrown) {
    UUIDGenerator generator;
    EXPECT_NO_THROW({
        auto uuid = generator.generateUUID();
        (void)uuid;
    });
}