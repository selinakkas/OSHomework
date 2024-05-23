#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROCESSES 100
#define RAM_SIZE 2048
#define RESERVED_RAM_FOR_CPU1 512
#define CPU2_QUANTUM_MEDIUM 8
#define CPU2_QUANTUM_LOW 16

// Process structure
typedef struct {
    char name[5];
    int arrival_time;
    int priority;
    int burst_time;
    int ram;
    int cpu_rate;
} Process;

// Function prototypes
void read_input_file(const char *filename, Process *processes, int *num_processes);
void assign_processes(Process *processes, int num_processes, FILE *output_file);
void print_cpu_queues(Process *processes, int num_processes);

void sort_processes_by_burst_time(Process *queue, int count);
void round_robin_scheduling(Process *queue, int count, int quantum, FILE *output_file);

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s input.txt\n", argv[0]);
        return 1;
    }

    Process processes[MAX_PROCESSES];
    int num_processes = 0;

    // Read input file
    read_input_file(argv[1], processes, &num_processes);

    // Open output file
    FILE *output_file = fopen("output.txt", "w");
    if (output_file == NULL) {
        perror("Error opening output file");
        return 1;
    }

    // Assign processes to CPUs
    assign_processes(processes, num_processes, output_file);

    // Print CPU queues
    print_cpu_queues(processes, num_processes);

    // Close output file
    fclose(output_file);

    return 0;
}

// Read input file and parse process details
void read_input_file(const char *filename, Process *processes, int *num_processes) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(1);
    }

    while (fscanf(file, "%[^,],%d,%d,%d,%d,%d\n", 
                  processes[*num_processes].name, 
                  &processes[*num_processes].arrival_time, 
                  &processes[*num_processes].priority, 
                  &processes[*num_processes].burst_time, 
                  &processes[*num_processes].ram, 
                  &processes[*num_processes].cpu_rate) != EOF) {
        (*num_processes)++;
    }

    fclose(file);
}

// Assign processes to CPUs
void assign_processes(Process *processes, int num_processes, FILE *output_file) {
    int ram_usage_cpu1 = 0;
    int ram_usage_cpu2 = 0;
    Process cpu1_queue[MAX_PROCESSES];
    Process cpu2_queue_priority1[MAX_PROCESSES];
    Process cpu2_queue_priority2[MAX_PROCESSES];
    Process cpu2_queue_priority3[MAX_PROCESSES];
    int cpu1_count = 0;
    int cpu2_count_priority1 = 0;
    int cpu2_count_priority2 = 0;
    int cpu2_count_priority3 = 0;

    for (int i = 0; i < num_processes; i++) {
        if (processes[i].priority == 0) {
            if (ram_usage_cpu1 + processes[i].ram <= RESERVED_RAM_FOR_CPU1) {
                cpu1_queue[cpu1_count++] = processes[i];
                ram_usage_cpu1 += processes[i].ram;
                fprintf(output_file, "Process %s is queued to be assigned to CPU-1.\n", processes[i].name);
            }
        } else {
            if (ram_usage_cpu2 + processes[i].ram <= (RAM_SIZE - RESERVED_RAM_FOR_CPU1)) {
                if (processes[i].priority == 1) {
                    cpu2_queue_priority1[cpu2_count_priority1++] = processes[i];
                } else if (processes[i].priority == 2) {
                    cpu2_queue_priority2[cpu2_count_priority2++] = processes[i];
                } else if (processes[i].priority == 3) {
                    cpu2_queue_priority3[cpu2_count_priority3++] = processes[i];
                }
                ram_usage_cpu2 += processes[i].ram;
                fprintf(output_file, "Process %s is queued to be assigned to CPU-2.\n", processes[i].name);
            }
        }
    }

    // Process CPU-1 queue (FCFS)
    for (int i = 0; i < cpu1_count; i++) {
        fprintf(output_file, "Process %s is assigned to CPU-1.\n", cpu1_queue[i].name);
        fprintf(output_file, "Process %s is completed and terminated.\n", cpu1_queue[i].name);
    }

    // Process CPU-2 queue (SJF, RR with different quantums)
    if (cpu2_count_priority1 > 0) {
        sort_processes_by_burst_time(cpu2_queue_priority1, cpu2_count_priority1);
        for (int i = 0; i < cpu2_count_priority1; i++) {
            fprintf(output_file, "Process %s is assigned to CPU-2 (Priority 1, SJF).\n", cpu2_queue_priority1[i].name);
            fprintf(output_file, "Process %s is completed and terminated.\n", cpu2_queue_priority1[i].name);
        }
    }

    if (cpu2_count_priority2 > 0) {
        round_robin_scheduling(cpu2_queue_priority2, cpu2_count_priority2, CPU2_QUANTUM_MEDIUM, output_file);
    }

    if (cpu2_count_priority3 > 0) {
        round_robin_scheduling(cpu2_queue_priority3, cpu2_count_priority3, CPU2_QUANTUM_LOW, output_file);
    }
}
