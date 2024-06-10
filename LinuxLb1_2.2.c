/*

2.2Напишите программу на языке C/C++, в которой:
1) Запускается четное количество потоков параллельно.
2) Каждый нечетный поток (например, первый) создает файл с именем из
своего PID, записывает в него произвольное число символов (от не-
скольких символов до сотен миллионов символов) и закрывает этот
файл.
3) Каждый четный поток (например, второй) открывает файл, созданный
предыдущим потоком с нечетным номером (в нашем случае первым),
читает его, считает количество символов в файле и закрывает его; при
этом четный поток не должен иметь никакой информации о количест-
ве записываемых в файл символов и о том, закончена ли запись в файл
нечетным потоком.
4) Каждый из потоков выводит следующую информацию: ThreadID, PID,
PPID, время, имя файла, количество записанных или считанных сим-
волов.
5) Количество пар создаваемых потоков передается аргументом в про-
грамму с командной строки. Программа должна ждать завершения
работы всех потоков, анализировать и сообщать о причинах заверше-
ния потоков. Для передачи имен файлом между потоками можно ис-
пользовать символьный массив в основной программе.

*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <string.h>

#define NKIDS 30


/*Структура kidrec используется для хранения информации о каждом потоке, включая его 
PID, TID, имя файла и количество символов*/
struct kidrec {
    pid_t pid;
    pthread_t tid;
    char filename[20];
    int num_chars;
}; struct kidrec kids [NKIDS];

void* odd_thread();
void* even_thread();
char* get_current_time() ;

int main(int argc, char* argv[]) {	
	
	// проверка корректности введённого аргумента
    int num_threads = atoi(argv[1]);
	if ( argc!=2 || num_threads%2!=0 ) {
        printf("Введённый аргумент должен быть единственным и быть чётным числом\n");
        exit(EXIT_FAILURE);
    }

    int i;
    pthread_t threads[num_threads];// массив потоков 
	
	// массив индексов от 1 до заданного кол-ва
    int indexes[num_threads]; 
    for (i = 1; i <= num_threads; i++) {
        indexes[i] = i;
        
		/* Для чётного индекса создаётся поток с выполнением ф-ции "even_thread"
		   Для нечётного  - ф-ции "odd_thread" */
        if (i % 2 != 0) {
            pthread_create(&threads[i], NULL, odd_thread, &indexes[i]);
        } else {
            pthread_create(&threads[i], NULL, even_thread, &indexes[i]);
        }
    }
    
    for (i = 1; i <= num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    
    return 0;
}
/////////////////////////////////////

/* функция, которая выполняется в нечетных потоках. Она принимает указатель на аргумент arg, 
который является индексом текущего потока в массиве indexes. Функция создает файл с именем, 
равным PID текущего процесса, записывает в него случайное количество символов от 'A' до 'Z'
и сохраняет информацию о PID, TID, имени файла и количестве записанных символов в структуре kids */
void* odd_thread(void* arg) {
    int index = *((int*) arg);
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    
	// сохраняет значение PID в поле filename структуры kids по индексу index. 
    sprintf(kids[index].filename, "%d", pid);
    
    FILE* file = fopen(kids[index].filename, "w");
    if (file == NULL) {
        printf("Error opening file for writing\n");
        pthread_exit(NULL);
    }
    
/* srand(time(NULL)) Используется для инициализации генератора случайных чисел rand(). Функция time(NULL) возвращает текущее время в секундах с начала эпохи (обычно 1 января 1970 года). Передача этого значения в функцию srand() позволяет установить начальное значение для генератора случайных чисел, основанное на текущем времени.
Использование srand(time(NULL)); обеспечивает более случайные результаты при генерации случайных чисел с помощью функции rand(). Без инициализации генератора случайных чисел, последовательность сгенерированных чисел будет одинаковой при каждом запуске программы. Использование текущего времени в качестве начального значения позволяет создавать разные последовательности случайных чисел при каждом запуске программы.*/  
  // srand(time(NULL));
    int num_chars = rand() % 1000000 + 1; // генерация случайного числа символов от 1 до 1 миллиона
    char* data = malloc(num_chars * sizeof(char)); //выделяется память для массива data рандомного размера num_chars*sizeof(char) символьного типа
    
	int i;
    for (i = 1; i <= num_chars; i++) {
        data[i] = 'A' + (rand() % 26); // генерация случайного символа от 'A' до 'Z' рандомное num_chars кол-во раз
    }
    
    fwrite(data, sizeof(char), num_chars, file); // Запись в файл
    
    fclose(file);
    
    kids[index].pid = pid;
    kids[index].tid = tid;
    kids[index].num_chars = num_chars;
    
	printf("№ %d,ThreadID: %lu, PID: %d, PPID: %d, Time: %s, Filename: %s, Записано: %d\n",
		index, kids[index].tid, kids[index].pid, getppid(), get_current_time() , kids[index].filename, kids[index].num_chars);
	
    free(data); //память освобождается
    
    pthread_exit(NULL);
}


/*это функция, которая выполняется в четных потоках. Она также принимает указатель 
на аргумент arg, который является индексом текущего потока в массиве indexes. 
Функция открывает файл, созданный предыдущим нечетным потоком, считывает количество 
символов из файла и сохраняет информацию о PID, TID, имени файла и количестве считанных 
символов в структуре kids.*/
void* even_thread(void* arg) {
    int index = *((int*) arg);
    pid_t pid = getpid();
    pthread_t tid = pthread_self();
    
    FILE* file = fopen(kids[index-1].filename, "r");
    if (file == NULL) {
        printf("Error opening file for reading\n");
        pthread_exit(NULL);
    }
    
    fseek(file, 0, SEEK_END);//устанавливает указатель положения файла file в конец файла. Третий аргумент SEEK_END указывает, что отсчет должен начинаться с конца файла. Это позволяет определить размер файла.
    int num_chars = ftell(file);//возвращает текущую позицию указателя положения файла file, которая теперь находится в конце файла после выполнения предыдущей функции fseek(). Значение num_chars будет равно количеству символов (байт) в файле.
    
    fclose(file);
    
    kids[index].pid = pid;
    kids[index].tid = tid;
    kids[index].num_chars = num_chars;
    
	printf("№ %d, ThreadID: %lu, PID: %d, PPID: %d, Time: %s, Filename: %s, Считано: %d\n",
		index, kids[index].tid, kids[index].pid, getppid(),  get_current_time() , kids[index].filename, kids[index].num_chars);
	
    pthread_exit(NULL);
}


 // вывод текущего времени
char* get_current_time() {
    time_t current_time;
    char* c_time_string;
    // Получаем текущее время
    current_time = time(NULL);
    // Преобразуем его в строку
    c_time_string = ctime(&current_time);
    // Разбиваем строку на отдельные части по пробелам
    char* token = (char*) strtok(c_time_string, " ");
    // Счетчик для определения нужных нам данных
    int count = 0;
    // Цикл для перебора всех частей строки
    while (token != NULL) {
        // Если счетчик равен 3, то это часы, минуты и секунды
        if (count == 3) {
            // Возвращаем строку с текущим временем
            return token;
        }
        // Увеличиваем счетчик и переходим к следующей части строки
        count++;
        token = (char*) strtok(NULL, " ");
    }
    return NULL;
} 
