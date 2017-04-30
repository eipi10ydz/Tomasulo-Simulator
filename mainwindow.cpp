#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdint>
#include <limits>

bool operator ==(const LoadBuffer &a, const LoadBuffer &b)
{
    return a.Qi == b.Qi && a.busy == b.busy && a.addr == b.addr && a.value == b.value;
}

bool operator ==(const Instruction &a, const Instruction &b)
{
    return a.name == b.name && a.op0 == b.op0 && a.op1 == b.op1 &&
            a.op2 == b.op2 && a.time_issue == b.time_issue &&
            a.time_exec_begin == b.time_exec_begin && a.time_exec_end == b.time_exec_end &&
            a.time_write_back == b.time_write_back;
}

bool operator ==(const InstructionState &a, const InstructionState &b)
{
    return a.Qi == b.Qi && a.state == b.state && a.execute_time == b.execute_time;
}

bool operator ==(const RegisterStation &a, const RegisterStation &b)
{
    return a.Qi == b.Qi && a.name == b.name && a.value == b.value;
}

bool operator ==(const ReservationStation &a, const ReservationStation &b)
{
    return a.time == b.time && a.Qi == b.Qi && a.Qj == b.Qj && a.Qk == b.Qk && a.Vj == b.Vj && a.Vk == b.Vk && a.op == b.op && a.busy == b.busy;
}


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    // 只能填大于0整数
    QRegExp regx("^[1-9][0-9]*$");
    QValidator *tmp = new QRegExpValidator(regx, ui->time_add_sub);
    validators.push_back(tmp);
    ui->time_add_sub->setValidator(tmp);
    tmp = new QRegExpValidator(regx, ui->time_div);
    validators.push_back(tmp);
    ui->time_div->setValidator(tmp);
    tmp = new QRegExpValidator(regx, ui->time_load);
    validators.push_back(tmp);
    ui->time_load->setValidator(tmp);
    tmp = new QRegExpValidator(regx, ui->time_mult);
    validators.push_back(tmp);
    ui->time_mult->setValidator(tmp);

    ui->cur_cycle->setEnabled(false);
    set_visible(false);
    set_enable(true);

    combo_boxs = std::vector<std::vector<QComboBox *>>(6, std::vector<QComboBox *>(4));
    combo_boxs[0][0] = ui->combo0_0;
    combo_boxs[0][1] = ui->combo0_1;
    combo_boxs[0][2] = ui->combo0_2;
    combo_boxs[0][3] = ui->combo0_3;
    combo_boxs[1][0] = ui->combo1_0;
    combo_boxs[1][1] = ui->combo1_1;
    combo_boxs[1][2] = ui->combo1_2;
    combo_boxs[1][3] = ui->combo1_3;
    combo_boxs[2][0] = ui->combo2_0;
    combo_boxs[2][1] = ui->combo2_1;
    combo_boxs[2][2] = ui->combo2_2;
    combo_boxs[2][3] = ui->combo2_3;
    combo_boxs[3][0] = ui->combo3_0;
    combo_boxs[3][1] = ui->combo3_1;
    combo_boxs[3][2] = ui->combo3_2;
    combo_boxs[3][3] = ui->combo3_3;
    combo_boxs[4][0] = ui->combo4_0;
    combo_boxs[4][1] = ui->combo4_1;
    combo_boxs[4][2] = ui->combo4_2;
    combo_boxs[4][3] = ui->combo4_3;
    combo_boxs[5][0] = ui->combo5_0;
    combo_boxs[5][1] = ui->combo5_1;
    combo_boxs[5][2] = ui->combo5_2;
    combo_boxs[5][3] = ui->combo5_3;

    registers = std::vector<QTableWidget *>(4, nullptr);
    registers[0] = ui->register_0;
    registers[1] = ui->register_1;
    registers[2] = ui->register_2;
    registers[3] = ui->register_3;

    for (int64_t i = 0; i < 6; ++i)
    {
        for (int64_t j = 1; j < 4; ++j)
            set_null(i, j);
    }

    instruction.resize(6);
    load_buffer.resize(1, std::vector<LoadBuffer>(3));
    fp_register.resize(1, std::vector<RegisterStation>(16));
    RS.resize(1, std::vector<ReservationStation>(5));
    instr_state.resize(1, std::vector<InstructionState>(6));
}

MainWindow::~MainWindow()
{
    delete ui;
    for (auto validator : validators)
        delete validator;
//    for (auto table_item : table_items)
//        delete table_item;
}

void MainWindow::set_visible(bool visible)
{
    ui->reservation->setVisible(visible);
    ui->register_0->setVisible(visible);
    ui->register_1->setVisible(visible);
    ui->register_2->setVisible(visible);
    ui->register_3->setVisible(visible);
    ui->load_part->setVisible(visible);
    ui->instr_state->setVisible(visible);
}

void MainWindow::set_enable(bool enable)
{
    for (int64_t i = 0; i < combo_boxs.size(); ++i)
    {
        for (int64_t j = 0; j < combo_boxs[i].size(); ++j)
        {
            combo_boxs[i][j]->setEnabled(enable);
        }
    }
    ui->time_add_sub->setEnabled(enable);
    ui->time_div->setEnabled(enable);
    ui->time_load->setEnabled(enable);
    ui->time_mult->setEnabled(enable);
    ui->start->setEnabled(enable);
    ui->exec->setEnabled(!enable);
    ui->exec_5->setEnabled(!enable);
    ui->back->setEnabled(!enable);
    ui->back_5->setEnabled(!enable);
}

bool MainWindow::check_start()
{
    if (ui->time_add_sub->text().isEmpty() || ui->time_div->text().isEmpty() || ui->time_load->text().isEmpty() || ui->time_mult->text().isEmpty())
        return false;
    bool flag = false;
    for (int64_t i = 0; i < combo_boxs.size(); ++i)
    {
        if (combo_boxs[i][0]->currentIndex() != 0)
        {
            flag = true;
            break;
        }
    }
    return flag;
}

void MainWindow::on_start_clicked()
{
    if (!check_start())
        return;
    set_visible(true);
    set_enable(false);
    ui->back->setEnabled(false);
    ui->back_5->setEnabled(false);
    get_time();
    init();
    instr_table_init();
    for (int64_t i = 0; i < 3; ++i)
    {
        load_buffer[0][i].busy = "no";
        load_buffer[0][i].Qi = "Load" + std::to_string(i + 1);
    }
    for (int64_t i = 0; i < 5; ++i)
    {
        RS[0][i].busy = "no";
        if (i < 3)
            RS[0][i].Qi = "Add" + std::to_string(i + 1);
        else
            RS[0][i].Qi = "MULT" + std::to_string(i - 2);
    }
    for (int64_t i = 0; i < 16; ++i)
    {
        fp_register[0][i].name = "F" + std::to_string(2 * i);
        fp_register[0][i].value = "R[" + fp_register[0][i].name + "]";
    }
}

void MainWindow::on_reset_clicked()
{
    set_visible(false);
    set_enable(true);
    reset();
}

void MainWindow::instr_table_init()
{
    for (int64_t i = 0; i < instruction_cnt; ++i)
    {
        ui->instr_state->setItem(i, 0, new QTableWidgetItem(convert(instruction[i]).c_str()));
    }
}

void MainWindow::get_time()
{
    add_sub_time = ui->time_add_sub->text().toLongLong();
    mult_time = ui->time_mult->text().toLongLong();
    div_time = ui->time_div->text().toLongLong();
    load_time = ui->time_load->text().toLongLong();
}

void MainWindow::set_register(int64_t row, int64_t column)
{
    QString tmp = "R";
    combo_boxs[row][column]->clear();
    for (int64_t i = 0; i <= 6; ++i)
    {
        combo_boxs[row][column]->insertItem(i, tmp + QString::number(i));
    }
}

void MainWindow::set_fp_register(int64_t row, int64_t column)
{
    QString tmp = "F";
    combo_boxs[row][column]->clear();
    for (int64_t i = 0; i < 16; ++i)
    {
        combo_boxs[row][column]->insertItem(i, tmp + QString::number(2 * i));
    }
}

void MainWindow::set_imm(int64_t row, int64_t column)
{
    combo_boxs[row][column]->clear();
    for (int64_t i = 0; i <= 7; ++i)
    {
        combo_boxs[row][column]->insertItem(i, QString::number(i));
    }
}

void MainWindow::set_null(int64_t row, int64_t column)
{
    combo_boxs[row][column]->clear();
    combo_boxs[row][column]->insertItem(0, "NULL");
}

void MainWindow::set_combo_box_row(int64_t row, int index)
{
    switch (index)
    {
        case NOP:
            for (int64_t i = 1; i < 4; ++i)
            {
                set_null(row, i);
            }
            break;
        case LD:
            set_fp_register(row, 1);
            set_imm(row, 2);
            set_register(row, 3);
            break;
        case ADDD:
        case SUBD:
        case MULTD:
        case DIVD:
            for (int64_t i = 1; i < 4; ++i)
            {
                set_fp_register(row, i);
            }
            break;
    }
}

void MainWindow::on_combo0_0_currentIndexChanged(int index)
{
    set_combo_box_row(0, index);
}

void MainWindow::on_combo1_0_currentIndexChanged(int index)
{
    set_combo_box_row(1, index);
}

void MainWindow::on_combo2_0_currentIndexChanged(int index)
{
    set_combo_box_row(2, index);
}

void MainWindow::on_combo3_0_currentIndexChanged(int index)
{
    set_combo_box_row(3, index);
}

void MainWindow::on_combo4_0_currentIndexChanged(int index)
{
    set_combo_box_row(4, index);
}

void MainWindow::on_combo5_0_currentIndexChanged(int index)
{
    set_combo_box_row(5, index);
}

void MainWindow::get_instr()
{
    for (int64_t i = 0; i < 6; ++i)
    {
        Instruction tmp;
        tmp.name = combo_boxs[i][0]->currentText().toStdString();
        if (tmp.name == "NOP")
            continue;
        tmp.op0 = combo_boxs[i][1]->currentText().toStdString();
        tmp.op1 = combo_boxs[i][2]->currentText().toStdString();
        tmp.op2 = combo_boxs[i][3]->currentText().toStdString();
        instruction[instruction_cnt] = tmp;
        int64_t &tmp_exec_time = instr_state[0][instruction_cnt++].execute_time;
        switch (combo_boxs[i][0]->currentIndex())
        {
            case LD:
                tmp_exec_time = load_time;
                break;
            case ADDD:
            case SUBD:
                tmp_exec_time = add_sub_time;
                break;
            case MULTD:
                tmp_exec_time = mult_time;
                break;
            case DIVD:
                tmp_exec_time = div_time;
                break;
        }
    }
}

std::string MainWindow::convert(const Instruction &instruction)
{
    std::string res;
    res += instruction.name;
    if (res == "L.D")
    {
        res += " ";
        res += instruction.op0;
        res += ", ";
        res += instruction.op1;
        res += "(";
        res += instruction.op2;
        res += ")";
    }
    else
    {
        res += " ";
        res += instruction.op0;
        res += ", ";
        res += instruction.op1;
        res += ", ";
        res += instruction.op2;
    }
    return res;
}

void MainWindow::reset()
{
    instruction_cnt = 0;
    cycles_cnt = 0;
    mem_cnt = 0;
    display_cycles_cnt = 0;
    back = false;
    finish = false;
    load_buffer.assign(1, std::vector<LoadBuffer>(3));
    fp_register.assign(1, std::vector<RegisterStation>(16));
    RS.assign(1, std::vector<ReservationStation>(5));
    instr_state.assign(1, std::vector<InstructionState>(6));
    display();
    ui->exec->setEnabled(false);
    ui->exec_5->setEnabled(false);
}

int64_t MainWindow::get_instr_classify(std::vector<int64_t> &to_be_exec, std::vector<int64_t> &exec, std::vector<int64_t> &to_be_write_back)
{
    int64_t res = -1;
    for (int64_t i = 0; i < instruction_cnt; ++i)
    {
        if (res == -1)
        {
            if (instr_state[cycles_cnt][i].state == TO_BE_ISSUE)
            {
                res = i;
            }
        }
        switch (instr_state[cycles_cnt][i].state)
        {
            case TO_BE_EXEC:
                to_be_exec.push_back(i);
                break;
            case EXEC:
                exec.push_back(i);
                break;
            case TO_BE_WRITEBACK:
                to_be_write_back.push_back(i);
                break;
        }
    }
    return res;
}

int64_t MainWindow::get_available_load()
{
    int64_t res = -1;
    for (int64_t i = 0; i < load_buffer[cycles_cnt].size(); ++i)
    {
        if (load_buffer[cycles_cnt][i].busy == "no")
        {
            res = i;
            break;
        }
    }
    return res;
}

int64_t MainWindow::get_available_reservation(int64_t instr_cnt)
{
    int64_t res = -1;
    if (instruction[instr_cnt].name == "ADD.D" || instruction[instr_cnt].name == "SUB.D")
    {
        for (int64_t i = 0; i < 3; ++i)
        {
            if (RS[cycles_cnt][i].busy == "no")
            {
                res = i;
                break;
            }
        }
    }
    else if (instruction[instr_cnt].name == "MULT.D" || instruction[instr_cnt].name == "DIV.D")
    {
        for (int64_t i = 3; i < 5; ++i)
        {
            if (RS[cycles_cnt][i].busy == "no")
            {
                res = i;
                break;
            }
        }
    }
    return res;
}

void MainWindow::process()
{
    if (finish)
        return;
    load_buffer.push_back(load_buffer[cycles_cnt]);
    fp_register.push_back(fp_register[cycles_cnt]);
    RS.push_back(RS[cycles_cnt]);
    instr_state.push_back(instr_state[cycles_cnt]);

    ++cycles_cnt;

    std::vector<int64_t> to_be_exec, exec, to_be_write_back;
    to_be_exec.reserve(6);
    exec.reserve(6);
    to_be_write_back.reserve(6);
    int64_t to_be_issued = get_instr_classify(to_be_exec, exec, to_be_write_back);


    // exec1
    for (int64_t i = 0; i < to_be_exec.size(); ++i)
    {
        InstructionState &instr_state_to_be_exec = instr_state[cycles_cnt][to_be_exec[i]];
        // load
        if (instruction[to_be_exec[i]].name == "L.D")
        {
            for (int64_t j = 0; j < 3; ++j)
            {
                if (load_buffer[cycles_cnt][j].Qi == instr_state_to_be_exec.Qi)
                {
                    load_buffer[cycles_cnt][j].addr = "R[" + instruction[to_be_exec[i]].op2;
                    if (instruction[to_be_exec[i]].op1 != "0")
                    {
                        load_buffer[cycles_cnt][j].addr += " + ";
                        load_buffer[cycles_cnt][j].addr += instruction[to_be_exec[i]].op1;
                    }
                    load_buffer[cycles_cnt][j].addr += "]";
                    --instr_state_to_be_exec.execute_time;
                    break;
                }
            }
            if (instr_state_to_be_exec.execute_time > 0)
            {
                instr_state[cycles_cnt][to_be_exec[i]].state = EXEC;
                if (!instruction[to_be_exec[i]].time_exec_begin)
                {
                    instruction[to_be_exec[i]].time_exec_begin = cycles_cnt;
                }
            }
            else
            {
                instr_state[cycles_cnt][to_be_exec[i]].state = TO_BE_WRITEBACK;
                instruction[to_be_exec[i]].time_exec_end = cycles_cnt;
            }
        }
        // fp
        else
        {
            for (int64_t j = 0; j < 5; ++j)
            {
                if (RS[cycles_cnt][j].Qi == instr_state_to_be_exec.Qi)
                {
                    if (RS[cycles_cnt][j].Qj.empty() && RS[cycles_cnt][j].Qk.empty())
                    {
                        --instr_state_to_be_exec.execute_time;
                        RS[cycles_cnt][j].time = instr_state_to_be_exec.execute_time;
                        if (instr_state_to_be_exec.execute_time > 0)
                        {
                            instr_state[cycles_cnt][to_be_exec[i]].state = EXEC;
                            if (!instruction[to_be_exec[i]].time_exec_begin)
                            {
                                instruction[to_be_exec[i]].time_exec_begin = cycles_cnt;
                            }
                        }
                        else
                        {
                            instr_state[cycles_cnt][to_be_exec[i]].state = TO_BE_WRITEBACK;
                            instruction[to_be_exec[i]].time_exec_end = cycles_cnt;
                        }
                    }
                    break;
                }
            }
        }
    }

    //exec2
    for (int64_t i = 0; i < exec.size(); ++i)
    {
        InstructionState &instr_state_exec = instr_state[cycles_cnt][exec[i]];
        // load
        if (instruction[exec[i]].name == "L.D")
        {
            for (int64_t j = 0; j < 3; ++j)
            {
                if (load_buffer[cycles_cnt][j].Qi == instr_state_exec.Qi)
                {
                    load_buffer[cycles_cnt][j].value = "M[" + load_buffer[cycles_cnt][j].addr + "]";
                    --instr_state_exec.execute_time;
                    break;
                }
            }
            if (instr_state_exec.execute_time == 0)
            {
                instr_state[cycles_cnt][exec[i]].state = TO_BE_WRITEBACK;
                instruction[exec[i]].time_exec_end = cycles_cnt;
            }
        }

        // fp
        else
        {
            for (int64_t j = 0; j < 5; ++j)
            {
                if (RS[cycles_cnt][j].Qi == instr_state_exec.Qi)
                {
                    --instr_state_exec.execute_time;
                    RS[cycles_cnt][j].time = instr_state_exec.execute_time;
                    break;
                }
            }
            if (instr_state_exec.execute_time == 0)
            {
                instr_state[cycles_cnt][exec[i]].state = TO_BE_WRITEBACK;
                instruction[exec[i]].time_exec_end = cycles_cnt;
            }
        }
    }

    // write back
    for (int64_t i = 0; i < to_be_write_back.size(); ++i)
    {
        instruction[to_be_write_back[i]].time_write_back = cycles_cnt;
        InstructionState &instr_state_to_be_write_back = instr_state[cycles_cnt][to_be_write_back[i]];
        // load
        if (instruction[to_be_write_back[i]].name == "L.D")
        {
            for (int64_t j = 0; j < 3; ++j)
            {
                if (load_buffer[cycles_cnt][j].Qi == instr_state_to_be_write_back.Qi)
                {
                    load_buffer[cycles_cnt][j].busy = "no";
                    load_buffer[cycles_cnt][j].addr = "";
                    load_buffer[cycles_cnt][j].value = "";
                    break;
                }
            }
        }

        // fp
        else
        {
            for (int64_t j = 0; j < 5; ++j)
            {
                if (RS[cycles_cnt][j].Qi == instr_state_to_be_write_back.Qi)
                {
                    RS[cycles_cnt][j].busy = "no";
                    RS[cycles_cnt][j].op = "";
                    RS[cycles_cnt][j].Qj = "";
                    RS[cycles_cnt][j].Qk = "";
                    RS[cycles_cnt][j].Vj = "";
                    RS[cycles_cnt][j].Vk = "";
                    break;
                }
            }
        }

        int64_t tmp_register = -1;
        for (int64_t j = 0; j < 16; ++j)
        {
            if (fp_register[cycles_cnt][j].Qi == instr_state_to_be_write_back.Qi)
            {
                fp_register[cycles_cnt][j].value = "M" + std::to_string(mem_cnt++);
                tmp_register = j;
                break;
            }
        }

        for (int64_t j = 0; j < 5; ++j)
        {
            if (RS[cycles_cnt][j].Qj == instr_state_to_be_write_back.Qi)
            {
                if (tmp_register != -1)
                    RS[cycles_cnt][j].Vj = fp_register[cycles_cnt][tmp_register].value;
                RS[cycles_cnt][j].Qj = "";
                // continue;
            }
            if (RS[cycles_cnt][j].Qk == instr_state_to_be_write_back.Qi)
            {
                if (tmp_register != -1)
                    RS[cycles_cnt][j].Vk = fp_register[cycles_cnt][tmp_register].value;
                RS[cycles_cnt][j].Qk = "";
            }
        }
        instr_state[cycles_cnt][to_be_write_back[i]].state = FINISH;
    }

    // issue
    if (to_be_issued != -1)
    {
        InstructionState &instr_state_issue = instr_state[cycles_cnt][to_be_issued];
        // load
        if (instruction[to_be_issued].name == "L.D")
        {
            int64_t available_load_pos = get_available_load();
            if (available_load_pos != -1)
            {
                instr_state_issue.Qi = load_buffer[cycles_cnt][available_load_pos].Qi;
                load_buffer[cycles_cnt][available_load_pos].busy = "yes";
                load_buffer[cycles_cnt][available_load_pos].addr = instruction[to_be_issued].op1;
                instruction[to_be_issued].time_issue = cycles_cnt;
            }
            else
                goto not_available;
        }

        // fp
        else
        {
            int64_t available_fp_pos = get_available_reservation(to_be_issued);
            if (available_fp_pos != -1)
            {
                instr_state_issue.Qi = RS[cycles_cnt][available_fp_pos].Qi;
                RS[cycles_cnt][available_fp_pos].busy = "yes";
                RS[cycles_cnt][available_fp_pos].op = instruction[to_be_issued].name;
                instruction[to_be_issued].time_issue = cycles_cnt;

                for (int64_t i = 0; i <= to_be_issued; ++i)
                {
                    if (instr_state[cycles_cnt][i].state == FINISH)
                        continue;
                    std::string des = instruction[i].op0;
                    int64_t op0_pos = std::stoll(des.substr(1)) / 2;
                    if (i != to_be_issued && instruction[to_be_issued].op1 == des)
                    {
                        RS[cycles_cnt][available_fp_pos].Qj = fp_register[cycles_cnt][op0_pos].Qi;
                        RS[cycles_cnt][available_fp_pos].Vj = "";
                    }
                    else if (instruction[i].name != "L.D")
                    {
                        if (RS[cycles_cnt][available_fp_pos].Qj.empty())
                        {
                            int64_t op1_pos = std::stoll(instruction[i].op1.substr(1)) / 2;
                            RS[cycles_cnt][available_fp_pos].Vj = fp_register[cycles_cnt][op1_pos].value;
                        }
                    }
                    if (i != to_be_issued && instruction[to_be_issued].op2 == des)
                    {
                        RS[cycles_cnt][available_fp_pos].Qk = fp_register[cycles_cnt][op0_pos].Qi;
                        RS[cycles_cnt][available_fp_pos].Vk = "";
                    }
                    else if (instruction[i].name != "L.D")
                    {
                        if (RS[cycles_cnt][available_fp_pos].Qk.empty())
                        {
                            int64_t op2_pos = std::stoll(instruction[i].op2.substr(1)) / 2;
                            RS[cycles_cnt][available_fp_pos].Vk = fp_register[cycles_cnt][op2_pos].value;
                        }
                    }
                }
            }
            else
                goto not_available;
        }
        int64_t des_reg_pos = std::stoll(instruction[to_be_issued].op0.substr(1)) / 2;
        fp_register[cycles_cnt][des_reg_pos].Qi = instr_state_issue.Qi;
        instr_state[cycles_cnt][to_be_issued].state = TO_BE_EXEC;
    }
    not_available:
        ;

    finish = true;
    for (int64_t i = 0; i < instruction_cnt; ++i)
    {
        if (instr_state[cycles_cnt][i].state != FINISH)
        {
            finish = false;
            break;
        }
    }
}

void MainWindow::display()
{
    if (finish)
    {
        ui->exec->setEnabled(false);
        ui->exec_5->setEnabled(false);
    }
    else
    {
        ui->exec->setEnabled(true);
        ui->exec_5->setEnabled(true);
    }
    if (cycles_cnt < 5)
    {
        ui->back_5->setEnabled(false);
    }
    else
    {
        ui->back_5->setEnabled(true);
    }
    if (cycles_cnt == 0)
    {
        ui->back->setEnabled(false);
    }
    else
    {
        ui->back->setEnabled(true);
    }
    ui->instr_state->clearContents();
    ui->reservation->clearContents();
    for (int64_t i = 0; i < 4; ++i)
    {
        registers[i]->clearContents();
    }
    ui->load_part->clearContents();

    ui->cur_cycle->setText(QString::number(cycles_cnt));

    for (int64_t i = 0; i < instruction_cnt; ++i)
    {
        ui->instr_state->setItem(i, 0, new QTableWidgetItem(convert(instruction[i]).c_str()));
    }

    for (int64_t i = 0; i < 3; ++i)
    {
        // if (cycles_cnt && load_buffer[cycles_cnt][i] == load_buffer[cycles_cnt-1][i])
        //     continue;
        ui->load_part->setItem(i, 0, new QTableWidgetItem(load_buffer[cycles_cnt][i].busy.c_str()));
        ui->load_part->setItem(i, 1, new QTableWidgetItem(load_buffer[cycles_cnt][i].addr.c_str()));
        ui->load_part->setItem(i, 2, new QTableWidgetItem(load_buffer[cycles_cnt][i].value.c_str()));
    }

    for (int64_t i = 0; i < 5; ++i)
    {
        // if (cycles_cnt && RS[cycles_cnt][i] == RS[cycles_cnt-1][i])
        //     continue;
        // todo: time
        if (RS[cycles_cnt][i].time)
            ui->reservation->setItem(i, 0, new QTableWidgetItem(QString::number(RS[cycles_cnt][i].time)));
        ui->reservation->setItem(i, 1, new QTableWidgetItem(RS[cycles_cnt][i].busy.c_str()));
        ui->reservation->setItem(i, 2, new QTableWidgetItem(RS[cycles_cnt][i].op.c_str()));
        ui->reservation->setItem(i, 3, new QTableWidgetItem(RS[cycles_cnt][i].Vj.c_str()));
        ui->reservation->setItem(i, 4, new QTableWidgetItem(RS[cycles_cnt][i].Vk.c_str()));
        ui->reservation->setItem(i, 5, new QTableWidgetItem(RS[cycles_cnt][i].Qj.c_str()));
        ui->reservation->setItem(i, 6, new QTableWidgetItem(RS[cycles_cnt][i].Qk.c_str()));
    }

    for (int64_t i = 0; i < instruction_cnt; ++i)
    {
        // if (cycles_cnt && instr_state[cycles_cnt][i] == instr_state[cycles_cnt-1][i])
        //     continue;
        if (instruction[i].time_issue && instruction[i].time_issue <= cycles_cnt)
        {
            ui->instr_state->setItem(i, 1, new QTableWidgetItem(std::to_string(instruction[i].time_issue).c_str()));
        }
        if (instruction[i].time_exec_begin && instruction[i].time_exec_begin <= cycles_cnt)
        {
            std::string tmp_str = std::to_string(instruction[i].time_exec_begin) + "~";
            if (instruction[i].time_exec_end && instruction[i].time_exec_end <= cycles_cnt)
                tmp_str += std::to_string(instruction[i].time_exec_end);
            ui->instr_state->setItem(i, 2, new QTableWidgetItem(tmp_str.c_str()));
        }
        if (instruction[i].time_write_back && instruction[i].time_write_back <= cycles_cnt)
        {
            ui->instr_state->setItem(i, 3, new QTableWidgetItem(std::to_string(instruction[i].time_write_back).c_str()));
        }
    }

    for (int64_t i = 0; i < 4; ++i)
    {
        for (int64_t j = 0; j < 4; ++j)
        {
            // if (cycles_cnt && fp_register[cycles_cnt][(i << 2) + j] == fp_register[cycles_cnt-1][(i << 2) + j])
            //     continue;
            registers[i]->setItem(0, j, new QTableWidgetItem(fp_register[cycles_cnt][(i << 2) + j].Qi.c_str()));
            if (fp_register[cycles_cnt][(i << 2) + j].value[0] != 'R')
                registers[i]->setItem(1, j, new QTableWidgetItem(fp_register[cycles_cnt][(i << 2) + j].value.c_str()));
        }
    }

}

void MainWindow::on_exec_clicked()
{
    if (!back)
    {
        process();
    }
    else if (cycles_cnt + 1 > real_cycles_cnt)
    {
        process();
    }
    else
    {
        ++cycles_cnt;
    }
    display();
}

void MainWindow::on_exec_5_clicked()
{
    if (!back)
    {
        for (int64_t i = 0; i < 5; ++i)
            process();
    }
    else if (cycles_cnt + 5 > real_cycles_cnt)
    {
        int64_t tmp = 5 - (real_cycles_cnt - cycles_cnt);
        cycles_cnt = real_cycles_cnt;
        for (int64_t i = 0; i < tmp; ++i)
            process();
    }
    else
    {
        cycles_cnt += 5;
    }
    display();
}

void MainWindow::init()
{
    get_instr();
}

void MainWindow::on_back_clicked()
{
    if (!back)
    {
        real_cycles_cnt = cycles_cnt;
        back = true;
    }
    finish = false;
    --cycles_cnt;
    if (cycles_cnt <= 0)
        cycles_cnt = 0;
    display();
}

void MainWindow::on_back_5_clicked()
{
    if (!back)
    {
        real_cycles_cnt = cycles_cnt;
        back = true;
    }
    finish = false;
    cycles_cnt -= 5;
    if (cycles_cnt <= 0)
        cycles_cnt = 0;
    display();
}
