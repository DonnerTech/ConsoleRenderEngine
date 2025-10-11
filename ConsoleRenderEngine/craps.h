#pragma once
#ifndef CRAPS_H
#define CRAPS_H

#include "physicsEngine.h"
#include "renderEngine.h"

void craps_game(void);

void print_game_rules(void);

double get_bank_balance(void);

double get_wager_amount(void);

int chech_wager_amount(double bank_balance, double wager_amount);

int roll_dice(void);

int get_dice_value(Body dice);

int is_win_loss_or_point(int dice_sum);

int is_point_loss_or_neither(int dice_sum, int point_value);

double adjust_bank_balance(double bank_balance, double wager_amount, int add_or_subtract);

void chatter_messages(int num_rolls, int win_loss_neither, double initial_bank_balance, double current_bank_balance);

#endif // CRAPS_H
