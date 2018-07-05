
#include "file_utils.h"
#include <gtest/gtest.h>

#define TEST_FILE       "test_file"


TEST (file_utils, save_to_file)
{
    // create file and write 4 bytes there
    int result = save_to_file (TEST_FILE, (void*)"test", 4);
    ASSERT_EQ (result, 0);

    // check the size
    FILE* f = fopen (TEST_FILE, "rb");
    fseek (f, 0, SEEK_END);
    int size = ftell (f);
    fseek (f, 0, SEEK_SET);
    fclose (f);
    ASSERT_EQ (size, 4);

    // after that delete it
    unlink (TEST_FILE);

    // invalid param
    result = save_to_file (NULL, (void*)"test", 4);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, read_from_file)
{
    // create empty test file
    //FILE* f = fopen (TEST_FILE, "wb");
    //fclose (f);
}

TEST (file_utils, drop_file)
{
    // create empty test file
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    // check that it was created
    struct stat buffer;
    int result = stat (TEST_FILE, &buffer);
    ASSERT_EQ (result, 0);

    // remove it
    drop_file (TEST_FILE);

    // check again
    result = stat (TEST_FILE, &buffer);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, drop_old_file)
{
}

TEST (file_utils, does_file_exist)
{
    // create empty test file
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    // and check that it exists
    int result = does_file_exist (TEST_FILE);
    ASSERT_EQ (result, 1);

    // after that delete it
    unlink (TEST_FILE);

    // and the routine shouldn't find it
    result = does_file_exist (TEST_FILE);
    ASSERT_EQ (result, 0);

    // negative
    result = does_file_exist (NULL);
    ASSERT_EQ (result, 0);
}
