#include <stdio.h>
#include <unistd.h>

void test_write()
{
    FILE *device = fopen("/dev/akhmadkhonov-driver", "r+");

    if (device == NULL)
    {
        printf("The device file is not opened.\n");
        return;
    }

    for (int i = 0; i < 11; ++i)
    {
        fputc(48 + i, device);
        printf("Wrote %d\n", i);
        fflush(device);
        usleep(200 * 1000);
    }

    fclose(device);
}

void test_read()
{
    FILE *device = fopen("/dev/akhmadkhonov-driver", "r");
    FILE *file_w = fopen("test.txt", "w");

    if (device == NULL)
    {
        printf("The device file is not opened.");
        return;
    }

    if (file_w == NULL)
    {
        printf("Could not open the output file");
        return;
    }

    int buffer;

    while ((buffer = fgetc(device)) != EOF)
    {
        // printing
        printf("%c", buffer);
        // putting in the file
        fputc(buffer, file_w);
    }

    fclose(device);
    fclose(file_w);
}

int main(void)
{
    test_write();
    return 0;
}