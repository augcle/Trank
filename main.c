#include <stdbool.h>
#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_WORD_COUNT 15000
#define MAX_SUCCESSOR_COUNT MAX_WORD_COUNT / 2

char book[] = {
#embed "example.txt" /// Stores the content of the file as an array of chars.
    , '\0'};      /// Makes `book` a string.

/// Array of tokens registered so far.
/// No duplicates are allowed.
char *tokens[MAX_WORD_COUNT];
/// `tokens`'s current size
size_t tokens_size = 0;

/// Array of successor tokens
/// One token can have many successor tokens. `succs[x]` corresponds to
/// `token[x]`'s successors.
/// We store directly tokens instead of token_ids, because we will directly
/// print them. If we wanted to delete the book, then it would make more sense
/// to store `token_id`s
char *succs[MAX_WORD_COUNT][MAX_SUCCESSOR_COUNT];
/// `succs`'s current size
size_t succs_sizes[MAX_WORD_COUNT] = {};

/// Overwrites non-printable characters in `book` with a space.
/// Non-printable characters may lead to duplicates like
/// `"\xefthe" and "the"` even both print `the`.
void replace_non_printable_chars_with_space() {
  int length = strlen(book);
  for(int i=0; i < length; i++){
    if(!isprint(book[i]))
      book[i] = ' ';}
}


/// Returns the id (index) of the token, creating it if necessary.
///
/// Returns token id if token exists in \c tokens, otherwise creates a new entry
/// in \c tokens and returns its token id.
///
/// \param token token to look up (or insert)
/// \return Index of `token` in \c tokens array
size_t token_id(char *token) {
  size_t id;
  for (id = 0; id < tokens_size; id++) {
    if (strcmp(tokens[id], token) == 0) {
      return id;
    }
  }
  tokens[id] = token;
  tokens_size++;
  return id;
}

/// Appends a token to the successors list of a `token`
void append_to_succs(char *token, char *succ) {
  size_t *next_empty_index_ptr = &succs_sizes[token_id(token)];

  if (*next_empty_index_ptr >= MAX_SUCCESSOR_COUNT) {
    fprintf(stderr, "Successor array full.");
    exit(EXIT_FAILURE);
  }

  succs[token_id(token)][(*next_empty_index_ptr)++] = succ;
}

void tokenize_and_fill_succs(char *delimiters, char *str) {
  // Tokenize the text and assign ID
  size_t tokenOrder[MAX_WORD_COUNT]; // store token ids => use size_t not char
  size_t tokenPlace = 0;

  char *token = strtok(str, delimiters);
  while (token) {
    size_t id = token_id(token);
    tokenOrder[tokenPlace] = id;
    tokenPlace++;
    token = strtok(NULL, delimiters);
  }

  if (tokenPlace < 2) {
    return;
  }

  // Creating the successor table
  char *succ;

  for (size_t i = 0; i < tokens_size; i++) {
    token = tokens[i];

    /* scan the token sequence positions (1..tokenPlace-1): if previous token
       equals `token` then tokenOrder[j] is a successor */
    for (size_t j = 1; j < tokenPlace; j++) {
      if (strcmp(token, tokens[tokenOrder[j - 1]]) == 0) {
        succ = tokens[tokenOrder[j]];

        if (succs_sizes[i] >= MAX_SUCCESSOR_COUNT) {
          continue;
        }

        append_to_succs(token, succ);
      }
    }
  }
}

/// Returns last character of a string
char last_char(char *str) {
  char lastChar = str[strlen(str)-1];
  return lastChar;
}

/// Returns whether the token ends with `!`, `?` or `.`.
bool token_ends_a_sentence(char *token) {
  if (last_char(token) == '!' || last_char(token) == '?' || last_char(token) == '.') {
    return true;
  }
  return false;
}

/// Returns a random `token_id` that corresponds to a `token` that starts with a
/// capital letter.
/// Uses \c tokens and \c tokens_size.

size_t random_token_id_that_starts_a_sentence() {
  int tokenID; // Create a variable
  while (1) { // Starts a loop that only stops when we find a uppercase letter
   tokenID = rand() % tokens_size; // Generate a random number, smaller than the size of the token array
  if (isupper(tokens[tokenID][0])) { // Check if the first letter of the randomly generated is upper case.
    break; // if it is, break. Else, generate again. 
    }
  }
  return tokenID; // When we exit the loop, return the ID. 
}

/// Generates a random sentence using \c tokens, \c succs, and \c succs_sizes.
/// The sentence array will be filled up to \c sentence_size-1 characters using
/// random tokens until:
/// - a token is found where \c token_ends_a_sentence
/// - or more tokens cannot be concatenated to the \c sentence anymore.
/// Returns the filled sentence array.
///
/// @param sentence array what will be used for the sentence.
//
//                  Will be overwritten. Does not have to be initialized.
/// @param sentence_size
/// @return input sentence pointer
char *generate_sentence(char *sentence, size_t sentence_size) {
  size_t current_token_id = random_token_id_that_starts_a_sentence();
  char *token = tokens[current_token_id];

  sentence[0] = '\0';

  strcat(sentence, token);
  if (token_ends_a_sentence(token))
    return sentence;

  // Calculated sentence length for the next iteration.
  // Used to stop the loop if the length exceeds sentence size
  size_t sentence_len_next;
  // Concatenates random successors to the sentence as long as
  // `sentence` can hold them.
  do {
    current_token_id = rand() % tokens_size;
    token = tokens[current_token_id];

    if (sentence[0] != '\0') {
      strncat(sentence, " ", sentence_size - strlen(sentence) - 1);
    }

    strcat(sentence, token);
    
    if (token_ends_a_sentence(token))
      return sentence;
  } while (sentence_len_next < sentence_size - 1);
  return sentence;
}

int main() {
  replace_non_printable_chars_with_space();

  char *delimiters = " \n\r";
  tokenize_and_fill_succs(delimiters, book);

  char sentence[10000];
  srand(time(NULL)); // Be random each time we run the program

  // Generate sentences until we find a question sentence.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '?');
  puts(sentence);
  puts("");

  // Initialize `sentence` and then generate sentences until we find a sentence
  // ending with an exclamation mark.
  do {
    generate_sentence(sentence, sizeof sentence);
  } while (last_char(sentence) != '!');
  puts(sentence);
}
