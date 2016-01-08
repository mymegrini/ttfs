#ifndef UTILS_H
#define UTILS_H
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
atou ( char *s );




/**
 * @brief This function waits for an answer from the user
 * @param[in] prompt message for the user
 * @return Returns 0 for [n/N] and 1 for [y/Y]
 *
 * This function displays a prompt message <prompt> and
 * waits for the user's answer and returns it
 */
int answer(char* prompt);


#endif /** UTILS_H */
