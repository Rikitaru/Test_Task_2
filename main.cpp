/*Требуется написать программу управления сборочным конвейером.
Конвейер состоит из движущейся ленты, на которой установлены детали,
и исполнительных механизмов(далее ИМ для краткости).
Конвейер разбит на участки, каждый из которых обслуживает отдельный ИМ.
Технологический цикл работы конвейера включает в себя сдвиг ленты и обработку деталей.
Сдвиг ленты возможен, только если все ИМ сообщили об успешном выполнении операции.
Если ИМ сообщает об аварии или не отвечает в течение заданного времени, то конвейер
останавливается и регистрируется авария, после чего возврат в автоматический режим
работы возможен только по команде оператора.
После сдвига ленты, ИМ по команде управляющей программы выполняет одну технологическую
операцию над деталью.После того как все ИМ успешно отработали операцию, технологический цикл повторяется.*/
#define IMPLEMENTER_CHANCE_ERROR 2              // вероятность ошибки ИМ при работе с деталью (в процентах)
#define IMPLEMENTER_NUMBER_OF_REPAIR_TRYING 2   // количество попыток починки ИМ
#define IMPLEMENTER_ACCEPTEBLE_DELAY 0.8        // допустимый отклик от ИМ (в секундах)
#define IMPLEMENTER_MAX_DELAY 1000              // максимально возможный отклик
#include <iostream>
#include <random>
#include <exception>
#include <stdexcept>
#include <iomanip>
#include <thread>

uint32_t RANDOM[100] = {0};
int RANDOM_I = 0;
enum states_Detail{         // состояния детали
    wait_detail = 0,                   // 0  Деталь ожидает работы над ней
    success_detail = 1,                // 1  Над деталью успешно поработали
    error_detail = -1         // -1 При работе над деталью случилась ошибка
};
class Detail {
private:
    int state;
public:
    Detail() {
        state = 0;
    }
    void set_state_Detail(int input_state) {
        state = input_state;
    }
};
class Timer {
private:
    // Псевдонимы типов используются для удобного доступа к вложенным типам
    using clock_t = std::chrono::high_resolution_clock;
    using second_t = std::chrono::duration<double, std::ratio<1> >;
    std::chrono::time_point<clock_t> m_beg;
public:
    Timer() : m_beg(clock_t::now()) {}

    void reset() {
        m_beg = clock_t::now();
    }

    double elapsed() const {
        return std::chrono::duration_cast<second_t>(clock_t::now() - m_beg).count();
    }
};
enum states{                // состояния для ИМ
    wait,                   // 0  ИМ ожидает деталь
    success,                // 1  ИМ успешно отработал с деталью
    busy,                   // 2  ИП на данном шаге будет работать над деталью
    error = -1,             // -1 ИМ получил ошибку, работая с деталью
    no_answer = -2,         // -2 ИП получил проблему "не отвечает в течение заданного времени", работая с деталью
};
class Conveyor {
private:
    class Implementer {
    private:
        int state; // состояние ИМ
        Detail *ptr_Detail_IM = nullptr;
    public:
        Implementer() {
            state = states(wait);
        }

        int get_state() const {
            return state;
        }

        void set_state(int input_state) {
            state = input_state;
        }

        void set_Detail(Detail *ptr) {
            ptr_Detail_IM = ptr;
        }
        int technological_operation() {
            if (state == states(busy)) {
                Timer t;
                uint32_t a = RANDOM[RANDOM_I++] % IMPLEMENTER_MAX_DELAY + 1;
                std::this_thread::sleep_for(std::chrono::milliseconds(a));
                double delta = t.elapsed();
                std::cout << "T: " << delta << " ";
                if (delta > IMPLEMENTER_ACCEPTEBLE_DELAY) {
                    state = states(no_answer);
                } else {
                    uint32_t b = RANDOM[RANDOM_I++] % IMPLEMENTER_MAX_DELAY + 1;
                    std::cout << " Er: " << b << " ";
                    state = (b <= IMPLEMENTER_CHANCE_ERROR) ? states(error) : states(success);
                }
                if (state == states(error)) {
                    ptr_Detail_IM->set_state_Detail(error_detail);
                }
                else if (state == states(success)){
                    ptr_Detail_IM->set_state_Detail(success_detail);
                }
                else {
                    ptr_Detail_IM->set_state_Detail(wait_detail);
                }
            } else {
                state = states(wait);
            }

            std::cout << state << " ";
            return state;
        }
        ~Implementer() {
            // delete ptr_Detail_IM;
            std::cout << "~Implementer" << std::endl;
        };
    };

    uint16_t number_implementers;           // количество ИМ в конвейере
    uint16_t number_details;                // количество Деталей в конвейере
    uint16_t step;                          // Номер Шага (цикла) конвейера
    Detail *ptr_Detail = nullptr;           // std::unique_ptr<Detail[]> ptr_Detail;
    Implementer *ptr_Implementer = nullptr; // std::unique_ptr<Implementer[]> ptr_Implementer;
public:
    Conveyor(int16_t const input_number_implementers, int16_t const input_number_details) : number_implementers(
            input_number_implementers), number_details(input_number_details),
                                                                                            step(0) { // конструктор с параметрами и исключениями
        if (input_number_implementers <= 0) {
            throw std::logic_error("Error. The input_number_implementers must be positive");
        }
        if (input_number_details <= 0) {
            throw std::logic_error("Error. The input_number_details must be positive");
        }
        ptr_Implementer = new Implementer[number_implementers]; // std::unique_ptr<Implementer[]>{new Implementer[number_implementers]};
        ptr_Detail = new Detail[number_details]; // ptr_Detail = std::unique_ptr<Detail[]>{new Detail[number_details]};
        show();
    }
    void process(int16_t const input_amount_steps) { //процесс конвейера
        if (input_amount_steps <= 0) {
            throw std::logic_error("Error. The input_amount_steps must be positive");
        }
        if (ptr_Implementer == nullptr) {
            throw std::logic_error("Error. Null ptr_Implementer");
        }
        do {
            uint16_t S = 0;
            std::cout << step << ": ";
            for (uint16_t i = 0; i <
                                 number_implementers; i++) { // в зависимости от номера Шага конвейера определяется занятость каждого ИМ
                if (step <= number_details) {
                    if (i < (step % (number_details + 1))) {
                        ptr_Implementer[i].set_state(states(busy)); // ИМ будет работаь на этом шаге
                        ptr_Implementer[i].set_Detail(
                                &ptr_Detail[step - i - 1]);  // Передаем ИМ деталь с которой он будет работать
                    }
                } else {
                    if (i < (step - number_details)) {
                        ptr_Implementer[i].set_state(states(wait)); // ИМ на этом шаге не будет работать
                    } else if (i < step) {
                        ptr_Implementer[i].set_Detail(&ptr_Detail[step - i - 1]);
                        ptr_Implementer[i].set_state(states(busy));
                    }
                }
            }
            std::thread MyThreads[number_implementers];
            Implementer *temp;
            for (int i = 0; i < number_implementers; i++) {
                temp = &ptr_Implementer[i];
                MyThreads[i] = std::thread([temp]() {
                    temp->technological_operation();
                });
            }
            for (int i = 0; i < number_implementers; i++) {
                MyThreads[i].join();
            }
            for (int i = 0; i < number_implementers; i++) {
                if ((ptr_Implementer[i].get_state() != states(error)) &&
                    (ptr_Implementer[i].get_state() != states(no_answer))) {
                    S++;
                }
            }
            std::cout << " S = " << S << std::endl;
            if (S != number_implementers) { // проверка на то, что все конвейеры отработали штатно
                if (!operators_reply()) { // полученгие решения о критической ситуации
                    std::cout << " pipeline stop" << std::endl;
                    return;
                } else {
                    if (repair(number_implementers - S) == 0) { // ремонт с несколькими попытками
                        std::cout << " pipeline stop" << std::endl;
                        return;
                    }
                }
            }
            step++;
            if ((step == input_amount_steps) && (step != 1) && (S == number_implementers)) {
                std::cout << " The pipeline has completed all the work" << std::endl;
            }
        } while (step != input_amount_steps);
    }

    int repair(int defective = 1) {
        int *ptr_repair_state = new int[defective];
        int j = 0;
        int flag = 2;
        for (int i = 0; i < number_implementers; i++) {
            int trying = 1;
            if ((ptr_Implementer[i].get_state() == states(error)) ||
                (ptr_Implementer[i].get_state() == states(no_answer))) {
                ptr_repair_state[j] = ptr_Implementer[i].get_state();
                std::cout << "Implementer [" << i << "] is state = " << ptr_repair_state[j] << " Repair.. "
                          << std::endl;
                ptr_Implementer[i].set_state(states(busy));
                ptr_repair_state[j] = ptr_Implementer[i].technological_operation();
                std::cout << "New state is " << ptr_repair_state[j] << std::endl;
                while (trying < IMPLEMENTER_NUMBER_OF_REPAIR_TRYING) {
                    if ((ptr_repair_state[j] == states(error)) || (ptr_repair_state[j] == states(no_answer))) {
                        ptr_Implementer[i].set_state(states(busy));
                        ptr_repair_state[j] = ptr_Implementer[i].technological_operation();
                        std::cout << "Trying again. New state is " << ptr_repair_state[j] << std::endl;
                    }
                    trying++;
                }
                if ((ptr_Implementer[i].get_state() != states(error)) &&
                    (ptr_Implementer[i].get_state() != states(no_answer))) {
                    std::cout << "Implementer as been fixed. An action has already been performed on the part."
                              << std::endl;
                    j++;
                    flag = 1;
                } else {
                    std::cout << "Implementer is faulty. Pipeline will be stopped." << std::endl;
                    flag = 0;
                    break;
                }
            }
        }
        delete[] ptr_repair_state;
        return flag;
    }

    void show() const {
        std::cout << "Number of implementers = " << number_implementers << std::endl;
        std::cout << "Number of details = " << number_details << std::endl;
    }

    uint16_t operators_reply() const {
        uint16_t answer = 0;
        std::cout << "There was an accident. Hardware error. In step " << get_step() << std::endl;
        while (true) {
            std::cout << "Continue automatic operation? (1 - Yes, 0 - No) " << std::endl;
            if (std::cin >> answer) {
                if (answer) {
                    break;
                } else {
                    std::cout << "Incorrect input, enter the number 1 - Yes, 0 - No" << std::endl;
                }
            } else {
                std::cin.clear();
                std::cin.ignore(32767, '\n');
                std::cout << "Incorrect input, enter the number 1 - Yes, 0 - No" << std::endl;
            }
        }
        return answer;
    }

    uint16_t get_step() const {
        return step;
    }

    ~Conveyor() {
        delete[] ptr_Implementer;
        delete[] ptr_Detail;
        std::cout << "~Conveyor" << std::endl;
    }
};

int main() {
    system("chcp 65001");
    std::random_device rd;
    std::mt19937 mersenne(rd()); // инициализируем Вихрь Мерсенна случайным стартовым числом
    for (unsigned int & i : RANDOM) {
        i = mersenne() % 1000;
        std::cout << i << " ";
    }
    std::cout << "MainId = " << std::this_thread::get_id() << std::endl;
    int16_t amount_steps = 0;
    std::cout << "How many steps should the pipeline perform?" << std::endl;
    std::cin >> amount_steps;
    try {
        Conveyor a(3, 5);
        a.process(amount_steps);
    }
    catch (std::exception const &e) {
        std::cerr << e.what() << '\n';
    }
    catch (...) {
        std::cout << "Unknown error" << '\n';
    }
    return 0;
}