/**
  ******************************************************************************
  * @file    help_func.h
  * @author  Cihan Senyuz
  * @brief   Header for help_func.c file.
  *                   This file contains the common defines of the application.
  *  
  ******************************************************************************
  */

/* gets the lenght of a string
*
* @param string that will be measured
* @return lenght of the string
*/
int str_len(char str[]);

/* finds a string in another string
*
* @param string being looked for, string needs to be checked
* @return 1 if exists, 0 if does not
*/
int str_find(char str1[],char str2[]);

/* checks if two string are exactly the same
*
* @param str1 first string
* @param str2 second string
* @return 1 if they are exact, 0 if are not
*/
int str_exact(char str1[],char str2[]);

/* concatenate two strings into one string
*
* @param str1 first string
* @param str2 second string to be added to first one
* @return new string that is created
*/
void str_merge(char str1[], char str2[]);

/* converts to string numbers from integer
*
* @param num number to be converted
* @return char* to new string that is created
* @note ATTENTION! this function allocates memory for returned char*
* Do not forget to deallocate it after you use.
*/
char* int2char(unsigned int num);

/* converts to integer from string numbers
*
* @param str string number to be converted
* @return new integer number that is created
*/
int char2int(char str[]);

/* clears the content of a char array
*
* @param str string to be cleared
* @return none
*/
void str_empty(char str[]);
