#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QValidator>
#include <QTableWidgetItem>
#include <QComboBox>
#include <vector>
#include <unordered_map>

const std::vector<std::string> instr_num_to_str = {"NOP", "L.D", "ADD.D", "SUB.D", "MULT.D", "DIV.D"};
const std::unordered_map<std::string, int64_t> instr_str_to_num = {{"NOP", 0}, {"L.D", 1}, {"ADD.D", 2}, {"SUB.D", 3}, {"MULT.D", 4}, {"DIV.D", 5}};
const int64_t NOP = 0;
const int64_t LD = 1;
const int64_t ADDD = 2;
const int64_t SUBD = 3;
const int64_t MULTD = 4;
const int64_t DIVD = 5;

const int64_t TO_BE_ISSUE = 0;
const int64_t TO_BE_EXEC = 1;
const int64_t EXEC = 2;
const int64_t TO_BE_WRITEBACK = 3;
const int64_t FINISH = 4;

struct LoadBuffer
{
    std::string Qi;
    std::string busy;
    std::string addr;
    std::string value;
};

struct Instruction
{
    std::string name;
    std::string op0;
    std::string op1;
    std::string op2;
    int64_t time_issue = 0;
    int64_t time_exec_begin = 0;
    int64_t time_exec_end = 0;
    int64_t time_write_back = 0;
};

struct InstructionState
{
    std::string Qi;
    int64_t state = 0;
    int64_t execute_time = 0;
};

struct RegisterStation
{
    std::string Qi;
    std::string name;
    std::string value;
};

struct ReservationStation
{
    int64_t time = 0;
    std::string Qi;
    std::string Qj;
    std::string Qk;
    std::string Vj;
    std::string Vk;
    std::string op;
    std::string busy;
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void set_visible(bool);
    void set_enable(bool);
    void instr_table_init();
    void set_register(int64_t row, int64_t column);
    void set_fp_register(int64_t row, int64_t column);
    void set_imm(int64_t row, int64_t column);
    void set_null(int64_t row, int64_t column);
    void set_combo_box_row(int64_t row, int index);
    bool check_start();
    void get_time();
    void get_instr();
    void reset();
    void process();
    void display();
    void init();
    int64_t get_available_load();
    int64_t get_available_reservation(int64_t);
    int64_t get_instr_classify(std::vector<int64_t>&, std::vector<int64_t>&, std::vector<int64_t>&);
    std::string convert(const Instruction&);

private slots:
    void on_start_clicked();
    void on_reset_clicked();

    void on_combo0_0_currentIndexChanged(int index);

    void on_combo1_0_currentIndexChanged(int index);

    void on_combo2_0_currentIndexChanged(int index);

    void on_combo3_0_currentIndexChanged(int index);

    void on_combo4_0_currentIndexChanged(int index);

    void on_combo5_0_currentIndexChanged(int index);

    void on_exec_clicked();

    void on_exec_5_clicked();

    void on_back_clicked();

    void on_back_5_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<QValidator *> validators;
    std::vector<QTableWidget *> registers;
    std::vector<std::vector<QComboBox *>> combo_boxs;
    std::vector<std::vector<LoadBuffer>> load_buffer;
    std::vector<std::vector<RegisterStation>> fp_register;
    std::vector<std::vector<ReservationStation>> RS;
    std::vector<Instruction> instruction;
    std::vector<std::vector<InstructionState>> instr_state;
    int64_t instruction_cnt = 0;
    int64_t cycles_cnt = 0;
    int64_t real_cycles_cnt = 0;
    int64_t display_cycles_cnt = 0;
    int64_t mem_cnt = 0;
    bool back = false;
    bool finish = false;
    int64_t add_sub_time;
    int64_t mult_time;
    int64_t div_time;
    int64_t load_time;
};

#endif // MAINWINDOW_H
