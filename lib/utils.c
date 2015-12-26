#include <ctype.h>
#include <string.h>
#include <inttypes.h>

#define STRBADCHAR (-1)
#define UINT32TOOBIG (-10)

/**
 * atou cast a string to a long long int destined to
 * to be a uint32_t.
 *
 * @param the string to cast
 * @return a long long int that fit in uint32_t or
 *         -1 if the string contains bad characters
 *         -10 if the string doesn't fit it uint32_t
 */
long long int
atou ( char *s ) {
  int size = strlen(s);
  uint32_t val = 0;
  int count = 0;
  uint32_t below_max = (UINT32_MAX - 1) / 10;
  while ( isdigit(*s) ) {
    if (val > below_max)
      return UINT32TOOBIG;
    val = val*10 + ( *s++ - '0' );
    ++count;
  }
  return (count==size)?val:STRBADCHAR;
}



#define YES 1
#define NO 0

/**
 * @brief This function waits for an answer from the user
 * @param[in] prompt message for the user
 * @return Returns 0 for [n/N] and 1 for [y/Y]
 *
 * This function displays a prompt message <prompt> and
 * waits for the user's answer and returns it
 */
int answer(char* prompt){
  printf("%s", prompt);
  switch(getchar()){
  case 'Y':	
  case 'y':
    return YES;
  case 'n':
  case 'N':
    return NO;
  default:
    while(getchar() != '\n');
    return answer(prompt);
  }
}

