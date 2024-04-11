#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int boardX, boardY, plrX, plrY, goalX, goalY, monX, monY;
char** board;

void printBoard()
{
    for (int y = boardY - 1; y >= 0; y--)
    {
        for (int x = 0; x < boardX - 1; x++)
        {
            printf("%c ", board[x][y]);
        }
        printf("%c\n", board[boardX-1][y]);
    }
}

int playerTurn(){
    if(feof(stdin)) return 1; 
    char s[5];
    fgets(s, 5, stdin); 
    if (s[0] == 'N'){
        if (plrY + 1 == boardY) {
            printf("invalid move\n");
            return 1;
        }else{
            board[plrX][plrY] = '.';
            plrY += 1;
            board[plrX][plrY] = 'P';
            return 0;
        }
    }else if(s[0] == 'S'){
        if (plrY - 1 < 0) {
            printf("invalid move\n");
            return 1;
        }else{
            board[plrX][plrY] = '.';
            plrY -= 1;
            board[plrX][plrY] = 'P';
            return 0;
        }
    }else if (s[0] == 'W'){
        if (plrX - 1 < 0) {
            printf("invalid move\n");
            return 1;
        }else{
            board[plrX][plrY] = '.';
            plrX -= 1;
            board[plrX][plrY] = 'P';
            return 0;
        }
    }else if (s[0] == 'E'){
        if (plrX + 1 == boardX) {
            printf("invalid move\n");
            return 1;
        }else{
            board[plrX][plrY] = '.';
            plrX += 1;
            board[plrX][plrY] = 'P';
            return 0;
        }
    }else{
        printf("invalid move\n");
        return 1;
    }
}

int monsterTurn(){
    int horizontalDistance = monX - plrX;
    int verticalDistance = monY - plrY;
    if (abs(horizontalDistance) > abs(verticalDistance)){
        //horizontal distance
        if (horizontalDistance < 0){
            //move east
            if(monX+1 != goalX || monY != goalY){
                board[monX][monY] = '.';
                monX += 1;
                board[monX][monY] = 'M';
                printf("monster moves E\n");
                return 0;
            }
            return 1;
        }else{
            //move west
            if(monX-1 != goalX || monY != goalY){
                board[monX][monY] = '.';
                monX -= 1;
                board[monX][monY] = 'M';
                printf("monster moves W\n");
                return 0;
            }
            return 1;
        }
    }else if (abs(verticalDistance) > abs(horizontalDistance)){
        //vertical move
        if (verticalDistance < 0){
            //move north
            if(monY+1 != goalY || monX != goalX){
                board[monX][monY] = '.';
                monY += 1;
                board[monX][monY] = 'M';
                printf("monster moves N\n");
                return 0;
            }
            return 1;
        }else{
            //move south
            if(monY-1 != goalY || monX != goalX){
                board[monX][monY] = '.';
                monY -= 1;
                board[monX][monY] = 'M';
                printf("monster moves S\n");
                return 0;
            }
            return 1;
        }
    }else{
        time_t t;
        srand((unsigned) time(&t));
        int dir = rand() % 2;
        if (dir == 1){
            if (verticalDistance < 0){
                //move north
                if(monY+1 != goalY || monX != goalX){
                    board[monX][monY] = '.';
                    monY += 1;
                    board[monX][monY] = 'M';
                    printf("monster moves N\n");
                    return 0;
                }
                return 1;
            }else{
                //move south
                if(monY-1 != goalY || monX != goalX){
                    board[monX][monY] = '.';
                    monY -= 1;
                    board[monX][monY] = 'M';
                    printf("monster moves S\n");
                    return 0;
                }
                return 1;
            }
        }else{
            if (horizontalDistance < 0){
                //move east
                if(monX+1 != goalX || monY != goalY){
                    board[monX][monY] = '.';
                    monX += 1;
                    board[monX][monY] = 'M';
                    printf("monster moves E\n");
                    return 0;
                }
                return 1;
            }else{
                //move west
                if(monX-1 != goalX || monY != goalY){
                    board[monX][monY] = '.';
                    monX -= 1;
                    board[monX][monY] = 'M';
                    printf("monster moves W\n");
                    return 0;
                }
                return 1;
            }
        }
    }

}

int main(int argc, char** argv)
{
    if(argc > 8)
    {
        boardX = atoi(argv[1]);
        boardY = atoi(argv[2]);
        plrX = atoi(argv[3]);
        plrY = atoi(argv[4]);
        goalX = atoi(argv[5]);
        goalY = atoi(argv[6]);
        monX = atoi(argv[7]);
        monY = atoi(argv[8]);
    }else{
        return 69;
    }

    //board = malloc(sizeof(char[boardX][boardY]));
    board = (char**) malloc (boardX*sizeof(char*));

    for (int x = 0; x < boardX; x++)
    {
        board[x] = (char*) malloc(boardY*sizeof(char));
        for (int y = 0; y < boardY; y++)
        {
            board[x][y] = '.';
        }
        //board[x] = row;
    }
    board[plrX][plrY] = 'P';
    board[monX][monY] = 'M';
    board[goalX][goalY] = 'G';

    while((plrY != monY || plrX != monX) && (plrY != goalY || plrX != goalX)){
        printBoard(board);
        int res = playerTurn();
        while(res == 1){
            if (!feof(stdin)){
                res = playerTurn();
            }else{
                return 1;
            }
        }
        if ((plrY == goalY && plrX == goalX) || (plrY == monY && plrX == monX))
        {
            break;
        }
        monsterTurn();
    }
    if (plrY == goalY && plrX == goalX){
        printf("player wins!\n");
    }else{
        printf("monster wins!\n");
    }
    for(int i = 0; i < boardX; i++){
        free(board[i]);
    }
    free(board);
}
