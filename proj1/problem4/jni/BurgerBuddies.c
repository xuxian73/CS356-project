/*
 * BurgerBuddies.c
 * Model scenario with cooks, cashiers, and customers.
 */
#include<stdio.h>
#include<stdlib.h>
#include<sys/syscall.h>
#include<unistd.h>
#include<pthread.h>
#include<semaphore.h>
#include<time.h>
#include<string.h>

sem_t sem_customer, sem_empty, sem_fill, sem_quit;
sem_t mutex;
int left;
int N_COOK, N_CASHIER, N_CUSTOMER, N_RACK;
pthread_t* cook_thread;
pthread_t* cashier_thread;
pthread_t* customer_thread;
pthread_t quit_thread;
int* cook_id;
int* cashier_id;
int* customer_id;

void *cook(void *argv) {
    int id = *(int *)argv;
    while (1)
    {
        sem_wait(&sem_empty);
        sleep(1);
        printf("Cook [%d] make a burger.\n", id);
        sem_post(&sem_fill);
    }
    return;
}

void *cashier(void *argv) {
    int id = *(int *)argv;
    while (1)
    {
        sem_wait(&sem_customer);
        printf("Cashier [%d] accepts an order.\n", id);
        sem_wait(&sem_fill);
        printf("Cashier [%d] take a burger to customer.\n", id);
        sem_post(&sem_empty);
        sem_wait(&mutex);
        --left;
        if(left == 0) {
            printf("quit triggered");
            sem_post(&sem_quit);
        }
        sem_post(&mutex);
    }
    return;
}

void *customer(void *argv) {
    int id = *(int *)argv;
    sleep(rand() % id);
    printf("Customer [%d] come.\n", id);
    sem_post(&sem_customer);
    return;
}

void *quit(void *argv) {
    sem_wait(&sem_quit);
    int i;
    for (i = 0; i < N_COOK; ++i) {
        /* pthread_kill(pthread_t thread, int sig) */
        pthread_kill(cook_thread[i], SIGQUIT);
    }
    for (i = 0;i < N_CASHIER; ++i) {
        pthread_kill(cashier_thread[i], SIGQUIT);
    }
    return;
}

int main(int argc, char* argv[]) {
    if (argc != 5) {
        printf("Usage: %s [-COOK] [-CASHIER] [-CUSTOMER] [-RACK].\n", argv[0]);
        return 0;
    }
    int i;
    N_COOK = atoi(argv[1]);
    N_CASHIER = atoi(argv[2]);
    N_CUSTOMER = atoi(argv[3]);
    N_RACK = atoi(argv[4]);
    left = N_CUSTOMER;
    printf("Cooks[%d], Cashiers[%d], Customers[%d]\n", N_COOK, N_CASHIER, N_CUSTOMER);
    
    /* sem_init(sem_t, int pshared, unsigned int value); */
    sem_init(&sem_customer, 0, 0);
    sem_init(&sem_empty, 0, N_RACK);
    sem_init(&sem_fill, 0, 0);
    sem_init(&sem_quit, 0, 0);
    sem_init(&mutex, 0, 1);
    
    cook_thread = malloc(sizeof(pthread_t) * N_COOK);
    cashier_thread = malloc(sizeof(pthread_t) * N_CASHIER);
    customer_thread = malloc(sizeof(pthread_t) * N_CUSTOMER);
    cook_id = malloc(sizeof(int) * N_COOK);
    cashier_id = malloc(sizeof(int) * N_CASHIER);
    customer_id = malloc(sizeof(int) * N_CUSTOMER);

    pthread_create(&quit_thread, NULL, quit, NULL);
    for (i = 0; i < N_COOK; ++i) {
        /* pthread_create(pthread_t* thread, const pthread_attr_t* attr,
           void *(*start_routine)(void*), void *arg);
        */
        cook_id[i] = i + 1;
        pthread_create(cook_thread + i, NULL, cook, cook_id + i);    
    }
    for (i = 0; i < N_CASHIER; ++i) {
        cashier_id[i] = i + 1;
        pthread_create(cashier_thread + i, NULL, cashier, cashier_id + i);
    }
    for (i = 0; i < N_CUSTOMER; ++i) {
        customer_id[i] = i + 1;
        pthread_create(customer_thread + i, NULL, customer, customer_id + i);
    }
    
    /* pthead_join(pthread_t, void** return); */
    pthread_join(quit_thread, NULL);
    for (i = 0; i < N_COOK; ++i) {
        pthread_join(cook_thread[i], NULL);
    }
    for (i = 0; i < N_CASHIER; ++i) {
        pthread_join(cashier_thread[i], NULL);
    }
    for (i = 0; i < N_CUSTOMER; ++i) {
        pthread_join(customer_thread[i], NULL);
    }

    sem_destroy(&sem_customer);
    sem_destroy(&sem_empty);
    sem_destroy(&sem_fill);
    sem_destroy(&sem_quit);
    sem_destroy(&mutex);

    free(cook_thread);
    free(cashier_thread);
    free(customer_thread);
    free(cook_id);
    free(cashier_id);
    free(customer_id);

    printf("All is done!");
}