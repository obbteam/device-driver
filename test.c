#include <stdio.h>

void test_driver()
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
    test_driver();
    return 0;
}