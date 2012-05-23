/**
 * Robert Adams 
 * May 16, 2012
 * Oregon State CS 475 proj7
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <pthread.h>
#define THREADS 4

const float GRAIN_GROWS_PER_MONTH =		9.0; // inches
const float ONE_DEER_EATS_PER_MONTH =		0.5;

const float AVG_PRECIP_PER_MONTH =		6.0; // inches
const float AMP_PRECIP_PER_MONTH =		6.0;
const float RANDOM_PRECIP =			2.0;

const float AVG_TEMP =				50.0; // farhrenheit
const float AMP_TEMP =				20.0;
const float RANDOM_TEMP =			10.0;

const float MIDTEMP =				40.0;
const float MIDPRECIP =				10.0;

int	NowYear;		// 2012 - 2017
int	NowMonth;		// 0 - 11

float	NowPrecip;		// inches of rain per month
float	NowTemp;		// temperature this month
float	NowHeight;		// grain height in inches
int NowBlood;
int	NowNumDeer;

pthread_barrier_t done_computing_barr;
pthread_barrier_t done_assigning_barr;
pthread_barrier_t done_printing_barr;
pthread_attr_t attr;
pthread_mutex_t calc;

void calc_temp_percipitation(void);
float		Ranf( float, float );
int		Ranf( int, int );

void *grain_growth(void *t) {
    int rc;
    float tempFactor;  
    float precipFactor; 
    while (true) {
        // do calculations
        precipFactor = exp(-1* pow(((float)(NowPrecip-MIDPRECIP)/10),2));
        tempFactor = exp(-1* pow(((float)(NowTemp-MIDTEMP)/10),2));

        rc = pthread_barrier_wait(&done_computing_barr);
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        // assign to global variable
        //printf("%f\t%f\t\n", precipFactor, tempFactor); // for debugging
        NowHeight = tempFactor * precipFactor * GRAIN_GROWS_PER_MONTH;
        NowHeight -= (float)NowNumDeer * ONE_DEER_EATS_PER_MONTH; 
        if(NowHeight < 0) NowHeight = 0;

        rc = pthread_barrier_wait(&done_assigning_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        rc = pthread_barrier_wait(&done_printing_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
    }
}

void *grain_deer(void *t) {
    int rc;
    int temp_num_deer;
    while (true) {
        // do calculations
        temp_num_deer = NowNumDeer; 
        if(NowHeight > temp_num_deer ) {
            temp_num_deer++;
        } else if( NowHeight < temp_num_deer) {
            temp_num_deer--;
        } 
        rc = pthread_barrier_wait(&done_computing_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        // assign to global variable
        NowNumDeer = temp_num_deer;
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            rc = pthread_barrier_wait(&done_assigning_barr); 
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        rc = pthread_barrier_wait(&done_printing_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
    }
}

void *grain_blood(void *t) {
    int rc;
    while (true) {
        rc = pthread_barrier_wait(&done_computing_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        // 1 in 15 chance of Exodus 7:14-25 
        if(Ranf(0,15) > 14) NowBlood = 4; // 3 days of blood

        if(NowBlood == 1) NowNumDeer--; // a deer dies
        if(NowBlood > 1) NowPrecip = 0; // rains blood, not water 
        if(NowBlood > 0) NowBlood--;
                        
        rc = pthread_barrier_wait(&done_assigning_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
    }
}

void *watcher(void *t) {
    int rc;
    int numMonths = 0;
    while (NowYear < 2018) {
        rc = pthread_barrier_wait(&done_assigning_barr);
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
        printf("%d\t%f\t%f\t%f\t%d\t%d\n", numMonths, NowPrecip, NowTemp, NowHeight, NowNumDeer, NowBlood);
        // increment time passed
        numMonths++;
        if(NowMonth < 12) NowMonth++;
        else {
            NowMonth = 0;
            NowYear++;
        }
        calc_temp_percipitation();

        int rc = pthread_barrier_wait(&done_printing_barr); 
        if(rc != 0 && rc != PTHREAD_BARRIER_SERIAL_THREAD) {
            printf("Could not wait on barrier\n");
            exit(-1);
        }
    }
    pthread_exit((void*) t);
}

void calc_temp_percipitation() {
    pthread_mutex_lock(&calc);
    float ang = (  30.*(float)NowMonth + 15.  ) * ( M_PI / 180. );
    float temp = AVG_TEMP - AMP_TEMP * cos( ang );
    NowTemp = temp + Ranf( -RANDOM_TEMP, RANDOM_TEMP );

    float precip = AVG_PRECIP_PER_MONTH + AMP_PRECIP_PER_MONTH * sin( ang );
    NowPrecip = precip + Ranf( -RANDOM_PRECIP, RANDOM_PRECIP );
    //printf("precip %f, now_precip %f\n", precip, NowPrecip); // for debugging
    if( NowPrecip < 0. )
        NowPrecip = 0.;
    //printf("precip %d, now_precip %d", precip, NowPrecip);
    pthread_mutex_unlock(&calc);
}

int main () {
    NowNumDeer = 1;
    NowHeight =  1.;
    NowMonth =    0;
    NowYear  = 2012;
    void *status;

    pthread_t threads[THREADS];
    long thread_ids[THREADS];
    for(int i = 0; i < THREADS; ++i) {
        thread_ids[i] = i;
    }
    pthread_mutex_init(&calc, NULL);
    pthread_barrier_init(&done_computing_barr, NULL, THREADS-1);
    pthread_barrier_init(&done_assigning_barr, NULL, THREADS-1);
    pthread_barrier_init(&done_printing_barr, NULL, THREADS-1);
    /* For portability, explicitly create threads in a joinable state */
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    calc_temp_percipitation();

    pthread_create(&threads[0], &attr, grain_growth, (void *) thread_ids[0]);
    pthread_create(&threads[1], &attr, grain_deer, (void *) thread_ids[1]);
    pthread_create(&threads[2], &attr, grain_blood, (void *) thread_ids[2]);
    pthread_create(&threads[3], &attr, watcher, (void *) thread_ids[3]);

    pthread_join(threads[3], &status); // join watcher thread
    pthread_cancel(threads[0]);
    pthread_cancel(threads[1]);
    pthread_cancel(threads[2]);

    pthread_attr_destroy(&attr);
    //pthread_exit(NULL);
    return 1;
}

float Ranf( float low, float high ) {
    float r = (float) rand();		// 0 - RAND_MAX
    return(   low  +  r * ( high - low ) / (float)RAND_MAX   );
}

int Ranf( int ilow, int ihigh ) {
    float low = (float)ilow;
    float high = (float)ihigh + 0.9999f;
    return (int)(  Ranf(low,high) );
}

