#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

int capacity = 0, occupancy = 0, SID = 0, studentstot = 0;
bool question = false, answer = false;
pthread_mutex_t q_mutex , a_mutex , o_mutex ,lock;
pthread_cond_t q_cond, a_cond, o_cond;

// Function that declares a question has been asked
void questionstart(int id)
{
    pthread_mutex_lock(&lock);
    pthread_mutex_lock(&q_mutex);
    SID = id;
    printf("Student %d asks a question.\n",SID);
    question = true;
    answer = false;
    
    pthread_cond_signal(&q_cond);
    pthread_mutex_unlock(&q_mutex);

    pthread_mutex_lock(&a_mutex);
    // Waits if there is no answer ready for the question
    while(!answer)
    {
        pthread_cond_wait(&a_cond, &a_mutex);
    }

    //pthread_cond_signal(&q_cond);
}

// Function that declares the question has been answered
void questiondone()
{
    printf("Student %d is satisfied.\n",SID);
    //pthread_mutex_unlock(&q_mutex);
    
    //pthread_mutex_lock(&lock);
    
    pthread_mutex_unlock(&a_mutex);
    pthread_mutex_unlock(&lock);
}

// Function for when the professor starts to answer a question
void answerstart()
{
    pthread_mutex_lock(&a_mutex);
    // Waits if there is no question being asked
    while(!question)
    {
        pthread_cond_wait(&q_cond, &a_mutex);
    }
    printf("Professor starts to answer question for Student %d.\n",SID);
}

// Function for when the professor finishes answering a question
void answerdone()
{
    printf("Professor is done with answer for Student %d.\n",SID);
    
    //pthread_mutex_lock(&lock);
    question =false;
    answer = true;
    //pthread_mutex_unlock(&lock);

    //pthread_mutex_unlock(&a_mutex);
    pthread_cond_signal(&a_cond);
    pthread_mutex_unlock(&a_mutex);

}

// Function for when a student enters the office
void  enter_office(int id)
{
    pthread_mutex_lock(&o_mutex);
    // Checks that the office is not overcrowded with too many students
    while(occupancy >= capacity)
    {
        pthread_cond_wait(&o_cond, &o_mutex);
    }

    occupancy++; // Increments occupancy
    printf("Student %d enters the office.\n",id);

    // Unlocks
    pthread_mutex_unlock(&o_mutex);
}

// Function for when a student leaves the office
void leave_office(int id)
{
    printf("Student %d leaves the office.\n",id);
    pthread_mutex_lock(&o_mutex);
    //printf("Student %d leaves the office.\n",SID);
    occupancy--; // Decrements occupancy

    pthread_cond_signal(&o_cond);

    // Unlocks
    pthread_mutex_unlock(&o_mutex);
}


void *student(void *id)
{
    //SID = (intptr_t) id;

    int *studentID;
    studentID = id;
    //printf("current studentID is %d.\n", *studentID);
    enter_office(*studentID);

    int num_questions = (*studentID%4) +1;

    for(int i = 1; i <= num_questions; i++)
    {
        questionstart(*studentID);
        questiondone(*studentID);
    }

    leave_office(*studentID);
}

void *professor()
{
    while(true)
    {
        answerstart();
        answerdone();
    }
}

// Main function
int main(int argc, char *argv[])
{
    pthread_mutex_init(&q_mutex, NULL);
    pthread_mutex_init(&o_mutex, NULL);
    pthread_mutex_init(&a_mutex, NULL);
    pthread_mutex_init(&lock, NULL);

    pthread_cond_init(&q_cond, NULL);
    pthread_cond_init(&o_cond, NULL);
    pthread_cond_init(&a_cond, NULL);

    //int studentstot;

    if (argc == 3) // Need to add number input verification
    {
        char *end;
        studentstot = atoi(argv[1]);
        capacity = atoi(argv[2]);

        pthread_t tstudent[studentstot];
        pthread_t tprofessor;

        int index[studentstot];
	// Creates a pthread for the professor
        pthread_create(&tprofessor, NULL, professor, NULL);
        
	// Creates a new student
        for(int i=0; i< studentstot; i++)
        {
            index[i] = i;
            pthread_create(&tstudent[i], NULL, student, (void*)&index[i]);
        }

        // Student enters the room
        for(int i =0; i < studentstot; i++)
        {
            pthread_join(tstudent[i], NULL);
        }

        // Sends a cancellation request to the thread thread
        pthread_cancel(tprofessor);

	// This destroys and initializes the condition variables
        pthread_mutex_destroy(&q_mutex);
        pthread_mutex_destroy(&o_mutex);
        pthread_mutex_destroy(&a_mutex);
        pthread_mutex_destroy(&lock);
        pthread_cond_destroy(&q_cond);
        pthread_cond_destroy(&o_cond);
        pthread_cond_destroy(&a_cond);
    }
    // Checks if the user entered valid integer values
    else
    {
        printf("Invalid Input(s)\n");
        return 2;
    }
}