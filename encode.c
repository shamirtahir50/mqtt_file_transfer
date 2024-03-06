#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>


#define PAGE_SIZE 1024 

// Define a struct for a page
typedef struct Page {
    char data[PAGE_SIZE];
    size_t size; // Actual size of data in this page
    SIMPLEQ_ENTRY(Page) entries; // Queue entries
} Page;

// Define a queue to hold pages
SIMPLEQ_HEAD(PageQueue, Page);

void bufferToPages(const char *buffer, struct PageQueue *pageQueue) {
    size_t bufferSize = strlen(buffer);
    size_t offset = 0;

    while (offset < bufferSize) {
        size_t remaining = bufferSize - offset;
        size_t chunkSize = remaining < PAGE_SIZE ? remaining : PAGE_SIZE;

        // Allocate a new page and populate its data
        Page *page = malloc(sizeof(Page));
        if (page == NULL) {
            perror("Failed to allocate memory for page");
            exit(EXIT_FAILURE);
        }
        memcpy(page->data, buffer + offset, chunkSize);
        page->size = chunkSize;

        // Enqueue the page
        SIMPLEQ_INSERT_TAIL(pageQueue, page, entries);

        offset += chunkSize;
    }
}

FILE *openFile(const char *filePath) {
    return fopen(filePath, "rb");
}
// Function to get the file size
size_t getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    return size;
}

// Function to read the file into a buffer
char *readFileIntoBuffer(FILE *file, size_t size) {
    char *buffer = (char *)malloc(size);
    if (buffer) {
        fread(buffer, 1, size, file);
    }
    return buffer;
}

int main(){

    printf("Reading file contents\n");
    const char *filePath = "./content.txt";
    
    // Use fopen to open the file
    FILE *file = openFile(filePath); // "r" mode for reading
    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }
    
    size_t fileSize = getFileSize(file);
    printf("Size: %zu\n", fileSize);

    char *buffer = readFileIntoBuffer(file, fileSize);
    if (buffer == NULL) {
        perror("Error reading file into buffer");
        fclose(file);
        return EXIT_FAILURE;
    }
    printf("Content: %s\n", buffer);

     // Initialize the page queue
    struct PageQueue pageQueue;
    SIMPLEQ_INIT(&pageQueue);

    // Convert buffer to pages and store in queue
    bufferToPages(buffer, &pageQueue);

    // Example: iterate over the queue and print page data
    Page *pagePtr;
    SIMPLEQ_FOREACH(pagePtr, &pageQueue, entries) {
        printf("Page content (size %zu):\n%.*s\n\n", pagePtr->size, (int)pagePtr->size, pagePtr->data);
    }

    // Cleanup: Don't forget to free the pages after processing
    while (!SIMPLEQ_EMPTY(&pageQueue)) {
        pagePtr = SIMPLEQ_FIRST(&pageQueue);
        SIMPLEQ_REMOVE_HEAD(&pageQueue, entries);
        free(pagePtr);
    }

    free(buffer);
    fclose(file); // Don't forget to close the file when you're done
    return 0;
}