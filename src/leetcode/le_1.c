/*********************************************************************************
  *Copyright(C),1996-2021, Company
  *FileName:  le_1.c
  *Author:  wyp
  *Version:  V1.0
  *Date:  2023-04-28 14:57:18
  *Description:


  *History:
     1.Date:
       Author:
       Modification:
     2.
**********************************************************************************/


/***************************************Includes***********************************/
//#include"xxx.h"
//#include"xxx.h"
#include <stdio.h>


/***************************************Macros***********************************/
//#define


/***************************************Variables***********************************/



/***************************************Functions***********************************/
/** 
 * @brief 
 * @file le_1.c 
 * @name 
 * @param[in] void 
 * @return void 
 * @note 
 * @date 2023-04-28 14:54:39 
 * @version V1.0.0 
*/
extern int leetcode_1_1_sum_of_two_numbers(void)
{

}

int* solution_1_1(int* nums, int numsSize, int target, int* returnSize, int* returnNum) 
{
    int i = 0;
    int j = 0;
    for (i = 0; i < numsSize; ++i) 
    {
        for (j = i + 1; j < numsSize; ++j) 
        {
            if (nums[i] + nums[j] == target) 
            {
                returnNum[0] = i, returnNum[1] = j;
                *returnSize = 2;
                return returnNum;
            }
        }
    }
    *returnSize = 0;
    return NULL;
}

/* [] END OF FILE */
