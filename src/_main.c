#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DIM 10 

int vs_ai;
int ai_turn_count;
int first_hit;
int hit[2];
int k[2];
int turn;
int put_phase;
int player;
int enemy_player;
int un[2];
int ship_decks[2][4] = {{0,0}};
int unkilled[2][10];
int ships_number[2];
int ready_fields;
char friends[2][DIM][DIM];
char enemies[2][DIM][DIM];

int check_borders(int x, int y);
int check_killed(int x, int y, int px, int py);
void mark_killed(int x, int y, int px, int py);
void init();
void fire(char inp[9]);
void put(int decks);
void put_ai_ships();
void put_parse_input(char inp[9]);
void populate();
void parse_input(char fn_inp[9], int ai_inp);
void tick();
void render();
void main_loop();


void populate() {
    friends[0][1][1] = '#';
    friends[0][1][2] = '#';
    friends[0][1][3] = '#';
    friends[0][1][4] = '#';

    friends[0][3][1] = '#';
    friends[0][3][2] = '#';
    friends[0][3][3] = '#';

    friends[0][6][3] = '#';
    friends[0][7][3] = '#';
    friends[0][8][3] = '#';

    friends[0][7][1] = '#';
    friends[0][8][1] = '#';

    friends[0][1][6] = '#';
    friends[0][1][7] = '#';

    friends[0][3][6] = '#';
    friends[0][3][7] = '#';

    friends[0][6][6] = '#';

    friends[0][6][8] = '#';

    friends[0][8][6] = '#';

    friends[0][8][8] = '#';
}

// init the field. args in case u need to clean only one field during a messed up population phase
void init() {
    k[0] = k[1] = 0;
    turn = 0;
    put_phase = 1;
    un[0] = un[1] = 0; //number of unkilled 
    hit[0] = hit[1] = 666;
    ships_number[0] = ships_number[1] = 0;
    
    for (int i=0; i<10; i++) {
        unkilled[0][i] = unkilled[1][i] = 666; // dud to sircimvent the check algorythm
    } 

    for (int y = 0; y < DIM; y++) {
        for (int x = 0; x < DIM; x++) {
            friends[0][x][y] = '.';
            enemies[0][x][y] = '.';
            friends[1][x][y] = '.';
            enemies[1][x][y] = '.';
        }
    }

    if (vs_ai) {
        put_ai_ships();
    }
    /*populate(enemies); // temporary*/
}

void q_switch(int q, int i, int arr[], int mark, char c) {
    int x, y;

    switch (q) {
        case 0: friends[1][x = arr[i] / 10][y = arr[i] % 10] = c; break;
        case 1: friends[1][x = arr[i] % 10][y = 9 - arr[i] / 10] = c; break;
        case 2: friends[1][x = 9 - arr[i] / 10][y = 9 - arr[i] % 10] = c; break;
        case 3: friends[1][x = 9 - arr[i] % 10][y = arr[i] / 10] = c; break;
    }

    if (mark) {
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[1][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    friends[1][i][j] = '/';
                }
            }
        }
    }
    /*printf("%i:%i ", x, y);*/
}

void put_ai_ships() {
    time_t t;
    srand((unsigned)time(&t));
    int i, x, y;
    int q = rand() % 4; // угол, в который будут сориентированны корабли
    /*q = 3;*/
    /*printf("%i\n", q);*/
    int decks[16] = {0,1,2,3,5,6,7,9,19,20,30,40,60,70,90,91};
    int duds[13] = {49,59,69,79,89,99,94,95,96,97,98, 39,93};

    for (i=0; i<16; i++) {
        q_switch(q, i, decks, 1, '#');
    }
    
    for (i=0; i<13; i++) {
        q_switch(q, i, duds, 0, '/');
    }

    for (i=0; i<3; i++) {
        /*printf("%i ", randd);*/
        q_switch(q, rand()%11, duds, 0, '.'); 
    }

    for (int i=0; i<4; i++) {
        x = rand() % 10;
        y = rand() % 10;
        while (friends[1][x][y] != '.') {
            x = rand() % 10;
            y = rand() % 10;
        }
        friends[1][x][y] = '#';

        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[1][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    friends[1][i][j] = '/';
                }
            }
        }
        /*printf("1(%i) %i:%i ", i, x, y);*/
    }
    /*printf("\n");*/
    ships_number[1] = 20;
    ready_fields = 1;
}


void fire(char inp[9]) {
    printf("fire()\n");
    if (!(inp[2] >= 'a' && inp[2] <= 'j' && inp[3] >= '0' && inp[3] <= '9')) {
        if (vs_ai && player == 1) { 
            printf("fire(): inp[] is \"%s\"\n", inp); 
        }
        else {
            printf("   Wrong target, try again.\n Player %i: ", player+1);
            parse_input("", 0); 
        }
        return;
    }

    int x = inp[2]-'a';
    int y = inp[3]-'0';

    switch (friends[enemy_player][x][y]) {
        case '#':
            enemies[player][x][y] = 'X';
            friends[enemy_player][x][y] = 'X';
            ships_number[enemy_player]--;
            /*printf("   ESNUM:%i (p%i)", ships_number[enemy_player], player+1);*/
            hit[player] = 10*x+y;

            int coord = 10*x + y;
            for (int i=0; i<10; i++) {
                if (unkilled[player][i] == coord) {
                    /*printf("\nfound one, coord = %i, un will be %i\n", coord, un[player]-1);*/
                    unkilled[player][i] = 0;
                    un[player]--;
                }
            }

            if (check_killed(x, y, x, y)) {
                /*printf("fire(): found killed in %i:%i\n", x, y);*/
                mark_killed(x, y, x, y);
                /*printf("\nKilled!\n");*/
            }

            /*if (vs_ai && player == 1) {*/
                /*printf("bot fired succesfully\n");*/
            /*}*/
            break;

        case '/':
        case '.':
            enemies[player][x][y] = '*';
            friends[enemy_player][x][y] = '*';
            hit[player] = 666;
            /*printf(" Miss.\n");*/
            break;

        default:
            if (vs_ai && player == 1) {
                printf("fire(): error. Target was %c (%i:%i)\n", friends[enemy_player][x][y], x, y);
                exit(0);
            } else {
                printf(" Wrong target, try again.\n Player %i: ", player+1);
                parse_input("", 0);
            }
            break;
    }
}


// returns 1 if borders are ok, 0 if not ok
int check_borders(int x, int y) {
    for (int i=x-1; i<=x+1; i++) {
        for (int j=y-1; j<=y+1; j++) {
            if (i<0 || j<0 || i >9 || j > 9) { 
                printf("%i %i\n", x, y, friends[player][i][j]);
                continue;
            }
            if (friends[player][i][j] == '#') {
                return 0;
            }
        }
    }
    return 1;
}


void parse_input(char fn_inp[9], int ai_inp) {
    char inp[9];
    if (vs_ai && player == 1) {
        for (int i=0; i<8; i++) {
            inp[i] = fn_inp[i];
            /*printf("inp[%i] = %c, fn_inp[%i] = %c\n", i, inp[i], i, fn_inp[i]);*/
        }
    } else {
        fgets(inp, 9, stdin);
    }

    switch (inp[0]) {
        case 'f':
            if (put_phase) {
                printf("   Invalid input: put phase isn't over.\n Player %i: ", player+1);
                parse_input("", 0);
            } else {
                fire(inp); // do this for all the other stuff 
            }
            break;

        case 'q':
            exit(0);
            break;
        
        case 'r':
            if (put_phase) {
                printf("\n    hi i reset\n");
                ships_number[player] = 0;
                ship_decks[player][0] = ship_decks[player][1] =
                ship_decks[player][2] = ship_decks[player][3] = 0;


                for (int y = 0; y < DIM; y++) {
                    for (int x = 0; x < DIM; x++) {
                        friends[player][x][y] = '.';
                    }
                }
                // reset the putting for current player somehow 
            } else {
                printf("   Invalid input: put phase is over.\n Player %i: ", player+1);
                parse_input("", 0);
            }
            break;

        case 'p':
            if (put_phase) {
                put_parse_input(inp);
            } else {
                printf("   Invalid input: put phase is over.\n Player %i: ", player+1);
                parse_input("", 0);
            }
            break;
        
        case 'o':
            populate();
            put_phase = 0;
            ready_fields = 2;
            ships_number[0] = 20;
            break;

        /*case 't':*/
            /*turn++;*/
            /*break;*/

        default:
            if (vs_ai && player == 1) {
                printf("error\n");
            } else {
                printf("   Invalid input, try again.\n Player %i: ", player+1);
                parse_input("", 0);
            }
            break;
    }
}


int check_killed(int x, int y, int px, int py) {
    printf("\ncheck_killed(): target was (%i %i)\n", x, y);

    for (int i=x-1; i<=x+1; i++) {
        for (int j=y-1; j<=y+1; j++) {
            if (i == x && j == y || (i<0 || i>9 || j<0 || j>9)) {
                printf("check_killed(): skipped (%i %i)\n", i, j);
                continue;
            }

            printf("check_killed():  (%i %i): %c\n", i, j, friends[enemy_player][i][j]);
            if (friends[enemy_player][i][j] == '#') {
                unkilled[player][k[player]++] = i*10+j;
                un[player]++;
                printf("\ncheck_killed(): un[%i]++ = %i\n", player, un[player]);
            }
            
            if (friends[enemy_player][i][j] == 'X' && i != px && j != py) {
                printf("check_killed(): inb4 recursive call\n");
                check_killed(i, j, x, y);
            }
        }
    }
    /*если он вернул не ноль, то там по какой-то причине не сработал счетчик неубитых*/
    printf("check_killed() has returned %i (un[%i]: %i)\n\n", !(un[player]), player, un[player]);
    return !(un[player]);
}


void mark_killed(int x, int y, int px, int py) {
    /*printf("\nmarking: %i %i, prev: %i %i\n", x, y, px, py);*/

    // to the left
    if (friends[enemy_player][x-1][y] == 'X' && x-1 != px) {
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[enemy_player][i][j] != 'X' && friends[enemy_player][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    enemies[player][i][j] = friends[enemy_player][i][j] = '*';
                }
            }
        }
        mark_killed(x-1, y, x, y);
    } 

    // to the right
    if (friends[enemy_player][x+1][y] == 'X' && x+1 != px) {
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[enemy_player][i][j] != 'X' && friends[enemy_player][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    enemies[player][i][j] = friends[enemy_player][i][j] = '*';
                }
            }
        }
        mark_killed(x+1, y, x, y);
    } 
    
    // upwards
    if (friends[enemy_player][x][y+1] == 'X' && y+1 != py) {
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[enemy_player][i][j] != 'X' && friends[enemy_player][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    enemies[player][i][j] = friends[enemy_player][i][j] = '*';
                }
            }
        }
        mark_killed(x, y+1, x, y);
    } 

    // downwards
    if (friends[enemy_player][x][y-1] == 'X' && y-1 != py) {
        for (int i=x-1; i<=x+1; i++) {
            for (int j=y-1; j<=y+1; j++) {
                if (friends[enemy_player][i][j] != 'X' && friends[enemy_player][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                    enemies[player][i][j] = friends[enemy_player][i][j] = '*';
                }
            }
        }
        mark_killed(x, y-1, x, y);
    } 

    for (int i=x-1; i<=x+1; i++) {
        for (int j=y-1; j<=y+1; j++) {
            if (friends[enemy_player][i][j] != 'X' && friends[enemy_player][i][j] != '#' && i>=0 && j>=0 && i<10 && j<10) {
                enemies[player][i][j] = friends[enemy_player][i][j] = '*';
            }
        }
    }
}

void ai_seek(char fire_str[9]) {
    int x, y;
    time_t t;
    srand((unsigned)time(&t));
    x = rand() % 10;
    y = rand() % 10;
    /*printf("player: %i, target: %c (%i:%i)\n", player, friends[0][x][y], x, y);*/
    while (friends[0][x][y] == '*' || friends[0][x][y] == 'X') {
        x = rand() % 10;
        y = rand() % 10;
        /*printf("player: %i, target: %c (%i:%i)\n", player, friends[0][x][y], x, y);*/
        /*usleep(1000000);*/
    }
    first_hit = x*10+y;
    fire_str[2] = 'a'+x;
    fire_str[3] = '0'+y;
}

void ai_finish(char fire_str[9]) {
    int x, y, chk = 1;
    printf("ai_finish():\n");
    x = hit[1] / 10;
    y = hit[1] % 10;
    printf("ai_finish(): hit[1]: %i\n", hit[1]);

    // left
    if (friends[0][x-1][y] != '*' && friends[0][x-1][y] != 'X' && x-1 >= 0) {
        /*printf("2\nh");*/
        if (friends[0][x-1][y] == '#') {
            if (y+1 <= 9) {
                friends[0][x][y+1] = '*';
                friends[0][x-1][y+1] = '*';
            }
            if (y-1 >= 0) {
                friends[0][x][y-1] = '*';
                friends[0][x-1][y-1] = '*';
            }
        }
        fire_str[2] = 'a'+x-1;
        fire_str[3] = '0'+y;
        chk = 0;
        /*printf("2\n");*/
    }
    // right
    if (friends[0][x+1][y] != '*' && friends[0][x+1][y] != 'X' && x+1 <= 9 && chk) {
        /*printf("3\n");*/
        if (friends[0][x+1][y] == '#') {
            if (y+1 <= 9) {
                friends[0][x][y+1] = '*';
                friends[0][x+1][y+1] = '*';
            }
            if (y-1 >= 0) {
                friends[0][x][y-1] = '*';
                friends[0][x+1][y-1] = '*';
            }
        }
        fire_str[2] = 'a'+x+1;
        fire_str[3] = '0'+y;
        chk = 0;
        /*printf("3\n");*/
    }
    // down
    if (friends[0][x][y-1] != '*' && friends[0][x][y-1] != 'X' && y-1 >= 0 && chk) {
        /*printf("4\n");*/
        if (friends[0][x][y-1] == '#') {
            if (x+1 <= 9) {
                friends[0][x+1][y] = '*';
                friends[0][x+1][y-1] = '*';
            }
            if (y-1 >= 0) {
                friends[0][x-1][y] = '*';
                friends[0][x-1][y-1] = '*';
            }
        }
        fire_str[2] = 'a'+x;
        fire_str[3] = '0'+y-1;
        chk = 0;
        /*printf("4\n");*/
    }
    // up
    if (friends[0][x][y+1] != '*' && friends[0][x][y+1] != 'X' && y+1 <= 9 && chk) {
        /*printf("5\n");*/
        if (friends[0][x][y+1] == '#') {
        if (x+1 <= 9) {
            friends[0][x+1][y] = '*';
            friends[0][x+1][y+1] = '*';
        }
        if (y-1 >= 0) {
            friends[0][x-1][y] = '*';
            friends[0][x-1][y+1] = '*';
        }
        }
        fire_str[2] = 'a'+x;
        fire_str[3] = '0'+y+1;
        /*printf("5\n");*/
    }
}

void tick() {
    int chk = 1;
    /*printf("\n%i %i %i\n", player, vs_ai, put_phase);*/
    /*printf("\ntick beg un[%i] = %i\n", player, un[player]);*/
    if (player == 1 && vs_ai && !put_phase) {
        if (ai_turn_count != 0) {
            char fire_str[] = "f e4\r"; 
            if (un[1] == 0) {
                ai_seek(fire_str);
            }
            if (hit[1] == 666 && un[1] != 0) {
                hit[1] = first_hit;
                printf("tick(): hit[1] set to %i\n", hit[1]);
            }
            if (hit[1] != 666 && un[1] != 0) {
                ai_finish(fire_str);
            }
            printf("tick(): firing now, fire_str: %s\n", fire_str);
            fire(fire_str);
        }
        ai_turn_count++;
    } else {
        parse_input("", 0);
    }
    /*printf("tick end un[%i] = %i\n", player, un[player]);*/
}


void render() {
    if (vs_ai && player == 1) {
        /*printf("No render\n");*/
        return;
    }
    int x, y;

    printf("\033[2J\033[1;1H");
    printf("\n    abcdefghij      abcdefghij\n");

    /*printf("\n    0123456789      0123456789  (debug)\n");*/
    
    for (y = 0; y < DIM; y++) {
        printf("  %i ", y);
        for (x = 0; x < DIM; x++) {
            printf("%c", friends[player][x][y]);
        }
        
        printf("    %i ", y);
        
        for (x = 0; x < DIM; x++) {
            printf("%c", enemies[player][x][y]);
        }
        printf("\n");
    }

    printf("\n Player %i: ", player+1);
}


void main_loop() {
    while (1) {
        player = turn % 2;
        enemy_player = (turn+1)%2;

        render();
        tick();
        render();

        if (put_phase && (ships_number[player] == 20 && ship_decks[player][0] == 4 && 
            ship_decks[player][1] == 3 && ship_decks[player][2] == 2 && 
            ship_decks[player][3] == 1))
        {
            if (!vs_ai) {
                turn++;
            }
            ready_fields++;
        }

        if (ready_fields == 2) {
            put_phase = 0;
        }

        if (hit[player] == 666 && !put_phase) {
            if (player == 0) {
                if (vs_ai && ai_turn_count == 0) {
                    printf("\n Press any key to begin the game. ");
                    getchar();
                } else {
                    if (vs_ai) {
                        /*printf("\n Press any key to let the enemy make a move. ");*/
                    } else {
                        printf("\n Press any key to switch sides. ");
                        getchar();
                    }
                }
            }
            turn++;
        }

        if (ships_number[enemy_player] == 0 && !put_phase) {
            printf("\n Player %i won!\n", player+1);
            exit(0);
        }
    }
}


void put_parse_input(char inp[9]) {
    int x = 0, y = 0, px = 0, py = 0;

    if (
        !(
            (inp[3] >= 'a' && inp[3] <= 'j' && inp[4] >= '0' && inp[4] <= '9') 
            && 
            (inp[5] >= 'a' && inp[5] <= 'j' && inp[6] >= '0' && inp[6] <= '9' || inp[1] == '1')
         )
    ) 
        // добавить проверку на порядок координат (b2b3, а не b3b2)
    {
        printf("   Invalid input, try again.\n Player %i: ", player+1);
        parse_input("", 0); 
        return;
    }

    switch (inp[1]) {
        case '1':
            if (ship_decks[player][0] >= 0 && ship_decks[player][0] < 4 && check_borders(inp[3]-'a', inp[4]-'0')) 
            {
                friends[player][inp[3]-'a'][inp[4]-'0'] = '#';
                ship_decks[player][0]++;
                ships_number[player]++;
            } else {
                printf("   Wrong placement, try again.\n Player %i: ", player+1);
                parse_input("", 0); 
            }
            break;

        case '2':
            if (ship_decks[player][1] >= 0 && ship_decks[player][1] < 3 && 
                check_borders(x = inp[3]-'a', y = inp[4]-'0') && 
                check_borders(px = inp[5]-'a', py = inp[6]-'0') &&
                (px-x <= 2 && py-y <= 2)) 
            {
                friends[player][x][y] = '#';
                friends[player][px][py] = '#';
                ship_decks[player][1]++;
                ships_number[player] += 2;
            } else {
                printf("   Wrong placement, try again.\n Player %i: ", player+1);
                parse_input("", 0); 
            }
            break;

        case '3':
            if (ship_decks[player][2] >= 0 && ship_decks[player][2] < 2 && 
                check_borders(x = inp[3]-'a', y = inp[4]-'0') && 
                check_borders(px = inp[5]-'a', py = inp[6]-'0') &&
                (px-x <= 3 && py-y <= 3)) 
            {
                friends[player][x][y] = '#';
                friends[player][x+(px-x)/2][y+(py-y)/2] = '#';
                friends[player][px][py] = '#';
                ship_decks[player][2]++;
                ships_number[player] += 3;
            } else {
                printf("1:%i 2:%i 3:%i 4:%i\n", ship_decks[player][0], ship_decks[player][1], ship_decks[player][2], ship_decks[player][3]);
                printf("   Wrong placement, try again.\n Player %i: ", player+1);
                parse_input("", 0); 
            }
            break;

        case '4':
            if (ship_decks[player][3] >= 0 && ship_decks[player][3] < 1 && 
                check_borders(x = inp[3]-'a', y = inp[4]-'0') && 
                check_borders(px = inp[5]-'a', py = inp[6]-'0') &&
                (px-x <= 4 && py-y <= 4)) 
            {
                friends[player][x][y] = '#';
                friends[player][x+(px-x)/2][y+(py-y)/2] = '#';
                friends[player][px-(px-x)/2][py-(py-y)/2] = '#';
                friends[player][px][py] = '#';
                ship_decks[player][3]++;
                ships_number[player] += 4;
            } else {
                if (ship_decks[player][3] >= 0 && ship_decks[player][3] < 1)
                    printf("1st");
                printf("   Wrong placement, %c try again.\n Player %i: ", friends[player][0][0],player+1);
                parse_input("", 0); 
            }
            break;
            
        default:
            printf("   Invalid input, try again.\n Player %i: ", player+1);
            parse_input("", 0);
            break;
    }
}


void main(int argc, char *argv[]) {
    vs_ai = 0;

    if(argc > 1 && argv[1][1] == 'a' && argv[1][2] == 'i') {
        vs_ai = 1;
    }

    init();
    main_loop();
}
