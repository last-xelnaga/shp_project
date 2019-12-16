
#include "file_utils.h"
#include <gtest/gtest.h>
using ::testing::InitGoogleTest;

#define TEST_FILE       "test_file"


TEST (file_utils, save_to_file_positive)
{
    // erase the test file as a precondition
    unlink (TEST_FILE);

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
}

TEST (file_utils, save_to_file_negative_01)
{
    // Permission denied
    int result = save_to_file ("/proc/cmdline", (void*)"test", 4);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, save_to_file_ivalid_param_01)
{
    // erase the test file as a precondition
    unlink (TEST_FILE);

    // invalid param
    int result = save_to_file (NULL, (void*)"test", 4);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, save_to_file_ivalid_param_02)
{
    // erase the test file as a precondition
    unlink (TEST_FILE);

    // invalid param
    int result = save_to_file (TEST_FILE, NULL, 4);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, save_to_file_ivalid_param_03)
{
    // erase the test file as a precondition
    unlink (TEST_FILE);

    // invalid param
    int result = save_to_file (TEST_FILE, (void*)"test", 10001);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, read_from_file_positive)
{
    // create a test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fwrite ("test", 1, 4, f);
    fclose (f);

    unsigned int size;
    void* data = NULL;
    int result = read_from_file (TEST_FILE, &data, &size);
    free (data);
    ASSERT_EQ (result, 0);
    ASSERT_EQ (size, (unsigned int)4);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, read_from_file_negative_01)
{
    // Permission denied
    unsigned int size;
    void* data = NULL;
    int result = read_from_file ("/root/.local", &data, &size);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, read_from_file_negative_02)
{
    // create an empty test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    unsigned int size;
    void* data = NULL;
    int result = read_from_file (TEST_FILE, &data, &size);
    ASSERT_EQ (result, -1);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, read_from_file_ivalid_param_01)
{
    // create a test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fwrite ("test", 1, 4, f);
    fclose (f);

    unsigned int size;
    void* data = NULL;
    int result = read_from_file (NULL, &data, &size);
    ASSERT_EQ (result, -1);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, read_from_file_ivalid_param_02_1)
{
    // create a test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fwrite ("test", 1, 4, f);
    fclose (f);

    unsigned int size;
    int result = read_from_file (TEST_FILE, NULL, &size);
    ASSERT_EQ (result, -1);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, read_from_file_ivalid_param_02_2)
{
    // create a test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fwrite ("test", 1, 4, f);
    fclose (f);

    unsigned int size;
    void* data = malloc (10);
    int result = read_from_file (TEST_FILE, &data, &size);
    ASSERT_EQ (result, -1);
    free (data);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, read_from_file_ivalid_param_03)
{
    // create a test file as a precondition
    FILE* f = fopen (TEST_FILE, "wb");
    fwrite ("test", 1, 4, f);
    fclose (f);

    void* data = NULL;
    int result = read_from_file (TEST_FILE, &data, NULL);
    ASSERT_EQ (result, -1);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils, drop_file_positive)
{
    // create an empty test file
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    // check that it was created
    struct stat buffer;
    int result = stat (TEST_FILE, &buffer);
    ASSERT_EQ (result, 0);

    // remove it
    result = drop_file (TEST_FILE);
    ASSERT_EQ (result, 0);

    // check again
    result = stat (TEST_FILE, &buffer);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, drop_file_negative_01)
{
    // Permission denied
    int result = drop_file ("/proc/cmdline");
    ASSERT_EQ (result, -1);
}

TEST (file_utils, drop_file_ivalid_param_01)
{
    // remove it
    int result = drop_file (NULL);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, drop_old_file_positive)
{
    // create an empty test file
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    int result = drop_old_file (TEST_FILE, 3);
    ASSERT_EQ (result, -1);

    // after that delete it
    unlink (TEST_FILE);
}

TEST (file_utils,  drop_old_file_negative_01)
{
    // create an empty test file
    FILE* f = fopen (TEST_FILE, "wb");
    fclose (f);

    sleep (5);

    int result = drop_old_file (TEST_FILE, 2);
    ASSERT_EQ (result, 0);
}

TEST (file_utils,  drop_old_file_negative_02)
{
    // Permission denied
    int result = drop_old_file ("/tmp/.X0-lock", 1);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, drop_old_file_ivalid_param_01)
{
    // remove it
    int result = drop_old_file (NULL, 1);
    ASSERT_EQ (result, -1);
}

TEST (file_utils, does_file_exist_positive)
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
}

TEST (file_utils, does_file_exist_ivalid_param_01)
{
    // negative
    int result = does_file_exist (NULL);
    ASSERT_EQ (result, 0);
}


int main (
        int argc,
        char **argv)
{
    InitGoogleTest (&argc, argv);
    return RUN_ALL_TESTS ();
}
