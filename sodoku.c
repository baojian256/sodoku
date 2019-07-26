#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum
{
    false = 0,
	true = 1
} bool_t;


// col mask
// +-------+-------+-------+
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// +-------+-------+-------+
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// +-------+-------+-------+
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// | 1 2 3 | 4 5 6 | 7 8 9 |
// +-------+-------+-------+
enum
{
    COL_MASK_NONE = 0x000, // 0 0000 0000
    COL_MASK_1 = 0x001,    // 0 0000 0001
    COL_MASK_2 = 0x002,    // 0 0000 0010
    COL_MASK_3 = 0x004,    // 0 0000 0100
    COL_MASK_4 = 0x008,    // 0 0000 1000
    COL_MASK_5 = 0x010,    // 0 0001 0000
    COL_MASK_6 = 0x020,    // 0 0010 0000
    COL_MASK_7 = 0x040,    // 0 0100 0000
    COL_MASK_8 = 0x080,    // 0 1000 0000
    COL_MASK_9 = 0x100,    // 1 0000 0000
    COL_MASK_123 = 0x007,  // 0 0000 0111
    COL_MASK_456 = 0x038,  // 0 0011 1000
    COL_MASK_789 = 0x1C0,  // 1 1100 0000
    COL_MASK_ALL = 0x1FF   // 1 1111 1111
};

typedef struct
{
    int base_col; // start col 0~8:valid, -1:invalid
    int curr_col; // current col 0~8:valid, -1:invalid
    unsigned int col_mask;
} col_index_t;

typedef struct{
    int long_row; // long_row 0~80:valid, -1:init, 81:finish
    col_index_t col_index[81]; // col_index[row]
} sodoku_t;


void init_sodoku(sodoku_t * sodoku);
bool_t sodoku_not_finish(sodoku_t *sodoku);
void goto_next_row(sodoku_t *sodoku);
void rollback_row(sodoku_t *sodoku);
int calc_rand_col(void);
int calc_next_col(int col);
void calc_col_mask_row(sodoku_t *sodoku, int long_row);
bool_t goto_next_col(sodoku_t *sodoku);
void print_sodoku(sodoku_t *sodoku);
		

int main(int argc, const char * argv[])
{
    sodoku_t sodoku;

    srand((unsigned int)time((time_t *)NULL));

    init_sodoku(&sodoku);
    goto_next_row(&sodoku);

    while(sodoku_not_finish(&sodoku))
    {
        if(goto_next_col(&sodoku))
        {
            goto_next_row(&sodoku);
        }
        else
        {
            rollback_row(&sodoku);
        }
    }

    print_sodoku(&sodoku);

    return 0;
}

void init_sodoku(sodoku_t * sodoku)
{
    do
    {
        if(sodoku == NULL)
            break;
 
        memset(sodoku, 0, sizeof(sodoku_t));
        
        sodoku->long_row = -1; // init invalid long_row
        
    } while(false);
}

bool_t sodoku_not_finish(sodoku_t *sodoku)
{
    bool_t not_finish = false;
    do
    {
        if(sodoku == NULL)
            break;

        if(sodoku->long_row <= 80)
        {
            not_finish = true;
        }

    } while(false);

    return not_finish;
}

void goto_next_row(sodoku_t *sodoku)
{
    do
    {
        if(sodoku == NULL)
            break;

        sodoku->long_row++;
        if(sodoku->long_row > 80)
            break;

        /* init col index */
        sodoku->col_index[sodoku->long_row].base_col = calc_rand_col(); // random col
        sodoku->col_index[sodoku->long_row].curr_col = -1; // invalid col

        /* init col mask */
        calc_col_mask_row(sodoku, sodoku->long_row);

    } while(false);
}

void rollback_row(sodoku_t *sodoku)
{
    do
    {
        if(sodoku == NULL)
            break;

        sodoku->long_row--;

        /* init col mask */
        calc_col_mask_row(sodoku, sodoku->long_row);

    } while(false);
}

int calc_rand_col(void)
{
    int col;
    col = rand() % 9; // col 0~8
    return col;
}

int calc_next_col(int col)
{
    return (col + 1) % 9; // col 012345678 => 123456780
}

void calc_col_mask_row(sodoku_t *sodoku, int long_row)
{
    int row;
    unsigned int mask = COL_MASK_NONE;

    // clear cell that already fill by other digit(in the same row)
    for(row = long_row % 9; row < long_row; row+=9)
    {
        mask |= sodoku->col_index[row].col_mask;
    }

    // clear cell that already fill by same digit(in the same col)
    for(row = long_row - long_row % 9; row < long_row; row++)
    {
        mask |= sodoku->col_index[row].col_mask;
    }

    // clear cell that already fill by same digit(in the same 3x3box)
    for(row = long_row - long_row % 3; row < long_row; row++)
    {
        if(sodoku->col_index[row].col_mask & COL_MASK_123)
            mask |= COL_MASK_123;
        else if(sodoku->col_index[row].col_mask & COL_MASK_456)
            mask |= COL_MASK_456;
        else // if(sodoku->col_index[row].col_mask & COL_MASK_789)
            mask |= COL_MASK_789;
    }

    sodoku->col_index[long_row].col_mask = ~mask; // store cols that can place
}

bool_t goto_next_col(sodoku_t *sodoku)
{
    bool_t success = false;
    int base_col;
    int curr_col;
    int next_col;
    bool_t found = false;

    do
    {
        if(sodoku == NULL)
            break;

        base_col = sodoku->col_index[sodoku->long_row].base_col;
        curr_col = sodoku->col_index[sodoku->long_row].curr_col;

        found = false;

        if(curr_col == -1)
        {
            // find first
            next_col = base_col;

            // base_col is valid
            if(sodoku->col_index[sodoku->long_row].col_mask & (1 << base_col))
            {
                found = true;

                next_col = base_col;
            }
            else
            {
                next_col = calc_next_col(base_col);
            }
        }
        else
        {
            next_col = calc_next_col(curr_col);
        }

        if(!found)
        {
            // find next
            while(next_col != base_col)
            {
                // next_col is valid
                if(sodoku->col_index[sodoku->long_row].col_mask & (1 << next_col))
                {
                    found = true;
                    break;
                }

                // find next
                next_col = calc_next_col(next_col);
            }
        }

        if(!found)
            break;

        // set next_col
        sodoku->col_index[sodoku->long_row].curr_col = next_col;
        sodoku->col_index[sodoku->long_row].col_mask = (1 << next_col); // store selected col

        success = true;
    } while(false);

    return success;
}

void print_sodoku(sodoku_t *sodoku)
{
    int long_row;
    int row;
    int col;
    int digit;
    int cell[81] = { 0 };
    do
    {
        if(sodoku == NULL)
            break;

        for(long_row = 0; long_row < sodoku->long_row; long_row++)
        {
            row = long_row % 9;
            col = sodoku->col_index[long_row].curr_col;
            digit = long_row / 9 + 1;

            cell[row * 9 + col] = digit;
        }

        printf("Sodoku %d\n", sodoku->long_row);

        for(row = 0; row < 9; row++)
        {
            if(row % 3 == 0)
                printf("+-----+-----+-----+\n");
            printf("|");
            for(col = 0; col < 9; col++)
            {
                printf("%d", cell[row * 9 + col]);
                if(col % 3 == 2)
                    printf("|");
                else
                    printf(" ");
            }
            printf("\n");
        }
        printf("+-----+-----+-----+\n");

    } while(false);
}

