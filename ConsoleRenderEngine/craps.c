/*
 * Description: This c file implements the functions for the craps gambling game.
 * Author: DonnerTech
 * Date: October 10, 2025
 */

#pragma warning(disable: 4996)

#include "craps.h"

void craps_game(void)
{
	// inits the renderer
	int init_status = userInit();
	if (init_status != 0)
	{
		return init_status;
	}

	print_game_rules();
	double bank_balance = get_bank_balance();
	double initial_bank_balance = bank_balance;

	while (bank_balance > 0)
	{
		double wager_amount = get_wager_amount();
		while (!chech_wager_amount(bank_balance, wager_amount))
		{
			wager_amount = get_wager_amount();
		}

		int dice_rolls = 1;

		printf("Rolling the dice...\n");
		int dice_sum = roll_dice();
		printf("You rolled a %d\n", dice_sum);
		int win_loss_or_point = is_win_loss_or_point(dice_sum);
		system("pause");

		if (win_loss_or_point == 1) // win
		{
			printf("You win!\n");
			bank_balance = adjust_bank_balance(bank_balance, wager_amount, 1);
		}
		else if (win_loss_or_point == 0) // loss
		{
			printf("Craps! You lose.\n");
			bank_balance = adjust_bank_balance(bank_balance, wager_amount, 0);
		}
		else // point
		{
			int point_value = dice_sum;

			printf("Your point is %d. Keep betting to make your point!\n", point_value);
			//wager_amount = get_wager_amount(); // ask for their new wager amount again

			while (win_loss_or_point == -1)
			{
				dice_rolls++;
				printf("Rolling the dice...\n");
				dice_sum = roll_dice();
				printf("You rolled a %d\n", dice_sum);
				win_loss_or_point = is_point_loss_or_neither(dice_sum, point_value);
				system("pause");
			}
			if (win_loss_or_point == 1) // made point
			{
				printf("You made your point! You win!\n");
				bank_balance = adjust_bank_balance(bank_balance, wager_amount, 1);
			}
			else if (win_loss_or_point == 0) // loss
			{
				printf("You rolled a 7 before making your point. You lose.\n");
				bank_balance = adjust_bank_balance(bank_balance, wager_amount, 0);
			}
		}
		printf("Your current bank balance is: $%.2f\n", bank_balance);

		// chatter msgs
		chatter_messages(dice_rolls, win_loss_or_point, initial_bank_balance, bank_balance);
		if (bank_balance <= 0)
		{
			printf("You've gone broke lmao! Game over...\n");
			break;
		}

		char choice;
		printf("Do you want to continue playing? (y/n): ");
		scanf(" %c", &choice);
		if (choice != 'y' && choice != 'Y')
		{
			printf("Thanks for playing! Come back again \n");
			break;
		}
	}


	end();

	return 0;
}

void print_game_rules(void)
{
	printf("Welcome to the game of Craps!\n\n");
	printf("Here are the rules:\n");
	printf("1. A player rolls two six-sided dice and adds the numbers rolled together.\n");
	printf("2. On the first roll, a sum of 7 or 11 wins, and a sum of 2, 3, or 12 loses (craps).\n");
	printf("3. If the sum is 4, 5, 6, 8, 9, or 10, that number becomes the player's \"point\".\n");
	printf("4. The player must continue rolling the dice until you can \"make your point\".\n");
	printf("5. The player loses by rolling a 7 before making your point\n");
	printf("Good luck and happy gambling! :>\n\n");
}

double get_bank_balance(void)
{
	printf("Enter your initial bank balance: $");
	double bank_balance = 0;
	scanf(" %lf", &bank_balance);

	return bank_balance;
}

double get_wager_amount(void)
{
	printf("Enter your wager amount: $");
	double wager_amount = 0;
	scanf(" %lf", &wager_amount);
	return wager_amount;
}

int chech_wager_amount(double bank_balance, double wager_amount)
{
	if (wager_amount > bank_balance)
	{
		printf("Wager amount cannot be greater than bank balance you brokie. Please enter a valid wager amount.\n");
		return 0;
	}
	if (wager_amount <= 0)
	{
		printf("Wager amount must be greater than zero bucko. Please enter a valid wager amount.\n");
		return 0;
	}
	return 1;
}

int roll_dice(void)
{
	PhysicsWorld world;

	// initialize the world with earth's gravity
	physicsWorld_Init(&world, (Vector3) { 0.0, 0.981, 0.0 });

	for (int i = 0; i < 2; i++)
	{

		//creates a dice box
		Vector3 half_extents = (Vector3){ 0.5, 0.5, 0.5 };
		Vector3 position = vector3_add((Vector3) { 0.0, -1.0 - 2 * half_extents.y * (i * 2 + 1), 4.0 }, vector3_scale(vector3_random(), 0.15));

		Vector3 randRot = vector3_random();
		Quaternion orientation = quat_from_euler(randRot.x * TWO_PI, randRot.y * TWO_PI, randRot.z * TWO_PI);
		orientation = quat_normalize(orientation);
		RigidBody box = rb_create_box(position, half_extents, orientation, 1.0);
		box.restitution = 0.2;
		physicsWorld_AddBody(&world, box);
	}


	// creates the ground plane
	RigidBody ground = rb_create_plane((Vector3) { 0.0, -1.0, 0.0 }, -2.5);
	ground.restitution = 0.999;
	physicsWorld_AddBody(&world, ground);

	bool isRunning = true;
	int tick = 0;
	double sim_frequency = 0;
	resetDeltaTime();

	// update loop
	while (isRunning)
	{
		blank(); // clear screen

		// update physics
		sim_frequency = 0;
		for (int i = 0; i < (int)deltaTime / 10 + 1; i++)
		{

			// update
			physicsWorld_Update(&world, 0.00016); // 1% realtime
			sim_frequency++;
		}

		//rendering
		renderer_raytrace(world.bodies, world.body_count, 60.0);

		// send frame to console
		renderFrame(); //16 = 60fps, 32 = 30fps

		//printf("tick: %d\n", tick);

		// frame timing
		printfFrameTimes(32, tick);

		tick++;

		// check if the esc key was pressed exit the loop
		if (_kbhit())
		{
			int input = _getch();
			if (input == 27)// 'esc' key
				isRunning = false;
		}

		// break when dice stop moving
		if (vector3_magnitude(world.rigidbodies[0].linearVelocity) < 0.1 && vector3_magnitude(world.rigidbodies[1].linearVelocity) < 0.1)
		{
			isRunning = false;
		}
		if (tick > 200)
		{
			isRunning = false;
		}
	}

	return get_dice_value(world.bodies[0]) + get_dice_value(world.bodies[1]);
}

int get_dice_value(Body dice)
{
	Vector3 cardinalDirections[6] = {
		{ 1, 0, 0 },  // 1
		{ 0, 1, 0 }, // 2
		{ 0, 0, -1 },  // 3
		{ 0, 0, 1 }, // 4
		{ 0, -1, 0 },  // 5
		{ -1, 0, 0 }  // 6
	};

	double maxDot = -1.0;
	int diceValue = 1;

	for (int i = 0; i < 6; i++)
	{
		Vector3 dir = quat_rotate_vector(dice.orientation, cardinalDirections[i]);

		double dot = vector3_dot(dir, (Vector3) { 0, -1, 0 }); // dot with up vector
	
		if(dot > maxDot)
		{
			maxDot = dot;
			diceValue = i + 1;
		}
	}

	return diceValue;
}

int is_win_loss_or_point(int dice_sum)
{
	if (dice_sum == 7 || dice_sum == 11)
	{
		return 1; // win
	}
	else if (dice_sum == 2 || dice_sum == 3 || dice_sum == 12)
	{
		return 0; // loss
	}
	else
	{
		return -1; // point
	}
}

int is_point_loss_or_neither(int dice_sum, int point_value)
{
	if (dice_sum == point_value)
	{
		return 1; // made point
	}
	else if (dice_sum == 7)
	{
		return 0; // loss
	}
	else
	{
		return -1; // neither
	}
}

double adjust_bank_balance(double bank_balance, double wager_amount, int add_or_subtract)
{
	if (add_or_subtract == 1)
	{
		bank_balance += wager_amount;
	}
	else if (add_or_subtract == 0)
	{
		bank_balance -= wager_amount;
	}
	return bank_balance;
}

void chatter_messages(int num_rolls, int win_loss_neither, double initial_bank_balance, double current_bank_balance)
{
	if (win_loss_neither == 1) // win
	{
		if (num_rolls == 1)
		{
			printf("What luck... Did you weigh the dice? \n");
		}
		else if (num_rolls <= 3)
		{
			printf("Not bad for a rookie \n");
		}
		else if (num_rolls <= 6)
		{
			printf("That sure took a while but you made it in the end! \n");
		}
		else
		{
			printf("You're one lucky gambler! \n");
		}
		if (current_bank_balance > initial_bank_balance)
		{
			printf("You're in the green! Time to go all in! \n");
		}
		else if (current_bank_balance < initial_bank_balance)
		{
			printf("You won, but you're still down overall. Better keep gambling kiddo\n");
		}
	}
	else if (win_loss_neither == 0) // loss
	{
		if (num_rolls == 1)
		{
			printf("Imagine losing on your first roll! \n");
		}
		else if (num_rolls <= 3)
		{
			printf("Tough luck mate. You almost had em! \n");
		}
		else if (num_rolls <= 6)
		{
			printf("What a game, to bad the house always comes on top! \n");
		}
		else
		{
			printf("%d rolls just to lose. What a tragedy \n", num_rolls);
		}
		if (current_bank_balance < initial_bank_balance && current_bank_balance > 0)
		{
			printf("You're down but not for the count, give it another shot! \n");
		}
		else if (current_bank_balance > initial_bank_balance)
		{
			printf("You're still green kid! \n");
		}
	}
}
