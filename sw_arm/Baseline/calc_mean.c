#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#define LEN 256

int main(int argc, char *argv[]) {
    double sum = 0;
    double mean;
    double min = DBL_MAX;
    double max = -DBL_MAX;
    char *buffer = (char *)malloc(LEN);
    double tmp_time;
    int mode = atoi(argv[1]);

    if (!mode) {
        FILE *fp = fopen("tmp_times.txt", "r");

        while(fgets(buffer, LEN, fp)) {
            tmp_time = atof(buffer);
            printf("%lf\n", tmp_time);
            buffer[0] = '\0';

            if (tmp_time < min) {
                min = tmp_time;
            }
            
            if (tmp_time > max) {
                max = tmp_time;
            }

            sum += tmp_time;
        }

        mean = (sum - min - max) / 10;
        printf("Mean execution time = %lfs\n", mean);
    }
    else {
        FILE *fp = fopen("tmp_times.txt", "r");

        while(fgets(buffer, LEN, fp)) {
            tmp_time = atof(buffer);
            buffer[0] = '\0';

            if (tmp_time < min) {
                min = tmp_time;
            }
            
            if (tmp_time > max) {
                max = tmp_time;
            }

            sum += tmp_time;
        }
        mean = (sum - min - max) / 10;
        printf("%lf\n", mean);
    }
    
}