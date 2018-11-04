#include "token.h"
#include "data.h"
#include "symbol.h"
#include <string.h>
#include <stdlib.h>

/**
 * This pointer is set when by parser() func and used by printParsedToken() func.
 * 
 * You are not required to use it anywhere. The implemented part of the skeleton
 * handles the printing. Instead, You could use the supplied helper functions to
 * manipulate the output file.
 * */
FILE* _out;

/**
 * Token list iterator used by the parser. It will be set once entered to parser()
 * and reset before exiting parser().
 * 
 * It is better to use the given helper functions to make use of token list iterator.
 * */
TokenListIterator _token_list_it;

/**
 * Current level.
 * */
unsigned int currentLevel;

/**
 * Symbol table.
 * */
SymbolTable symbolTable;

/**
 * Returns the current token using the token list iterator.
 * If it is the end of tokens, returns token with id nulsym.
 * */
Token getCurrentToken();

/**
 * Returns the type of the current token. Returns nulsym if it is the end of tokens.
 * */
int getCurrentTokenType();

/**
 * Prints the given token on _out by applying required formatting.
 * */
void printCurrentToken();

/**
 * Advances the position of TokenListIterator by incrementing the current token
 * index by one.
 * */
void nextToken();

/**
 * Given an entry from non-terminal enumaration, prints it.
 * */
void printNonTerminal(NonTerminal nonTerminal);

/**
 * Functions used for non-terminals of the grammar
 * */
int program();
int block();
int const_declaration();
int var_declaration();
int proc_declaration();
int statement();
int condition();
int relop();
int expression();
int term();
int factor();

Token getCurrentToken()
{
    return getCurrentTokenFromIterator(_token_list_it);
}

int getCurrentTokenType()
{
    return getCurrentToken().id;
}

void printCurrentToken()
{
    fprintf(_out, "%8s <%s, '%s'>\n", "TOKEN  :", tokenNames[getCurrentToken().id], getCurrentToken().lexeme);
}

void nextToken()
{
    _token_list_it.currentTokenInd++;
}

void printNonTerminal(NonTerminal nonTerminal)
{
    fprintf(_out, "%8s %s\n", "NONTERM:", nonTerminalNames[nonTerminal]);
}

/**
 * Given the parser error code, prints error message on file by applying
 * required formatting.
 * */
void printParserErr(int errCode, FILE* fp)
{
    if(!fp) return;

    if(!errCode)
        fprintf(fp, "\nPARSING WAS SUCCESSFUL.\n");
    else
        fprintf(fp, "\nPARSING ERROR[%d]: %s.\n", errCode, parserErrorMsg[errCode]);
}

/**
 * Advertised parser function. Given token list, which is possibly the output of 
 * the lexer, parses the tokens. If encountered, return the error code.
 * 
 * Returning 0 signals successful parsing.
 * Otherwise, returns a non-zero parser error code.
 * */
int parser(TokenList tokenList, FILE* out)
{
    // Set output file pointer
    _out = out;

    /**
     * Create a token list iterator, which helps to keep track of the current
     * token being parsed.
     * */
    _token_list_it = getTokenListIterator(&tokenList);

    // Initialize current level to 0, which is the global level
    currentLevel = 0;

    // Initialize symbol table
    initSymbolTable(&symbolTable);

    // Write parsing history header
    fprintf(_out, "Parsing History\n===============\n");

    // Start parsing by parsing program as the grammar suggests.
    int err = program();

    // Print symbol table - if no error occured
    if(!err)
    {
        fprintf(_out, "\n\n");
        printSymbolTable(&symbolTable, _out);
    }

    // Reset output file pointer
    _out = NULL;

    // Reset the global TokenListIterator
    _token_list_it.currentTokenInd = 0;
    _token_list_it.tokenList = NULL;

    // Delete symbol table
    deleteSymbolTable(&symbolTable);

    // Return err code - which is 0 if parsing was successful
    return err;
}

int program()
{
    printNonTerminal(PROGRAM);
	
	// Error variable to track errors.
	int err = 0;
	
	// Pass to block and check error code returned.
	err = block();
	if(err != 0)
		return err;
	
	// Check if the last symbol is a period, otherwise return
	// error 6 for "period expected".
	if(getCurrentTokenType() != periodsym)
		return 6;
	
	// Print period.
	printCurrentToken();

    return 0;
}

int block()
{
    printNonTerminal(BLOCK);
	
	// Error variable to track errors.
	int err = 0;
	
	// Check if current token is a constant and pass to constant
	// declaration.
	printNonTerminal(CONST_DECLARATION);
    if(getCurrentTokenType() == constsym && err == 0)
		err = const_declaration();
	// Error check
	if(err != 0)
		return err;
	
	// Check if current token is a variable and pass to variable
	// declaration.
	printNonTerminal(VAR_DECLARATION);
    if(getCurrentTokenType() == varsym && err == 0)
		err = var_declaration();
	// Error check
	if(err != 0)
		return err;
	
	// Check if current token is a procedure and pass to 
	// procedure declaration.
	printNonTerminal(PROC_DECLARATION);
	if(getCurrentTokenType() == procsym && err == 0)
		err = proc_declaration();
	// Error check
	if(err != 0)
		return err;
	
	err = statement();
	
    return err;
}

int const_declaration()
{
	// Do while loop parses constant declaration. Goes until a 
	// comma isn't found.
    do
	{
		// Declare a new Symbol and set its type and level 
		// values.
		Symbol newSym;
		newSym.type = CONST;
		newSym.level = currentLevel;
		
		// Get next token and check that it is an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;
		// Update the symbol's name.
		strcpy(newSym.name, getCurrentToken().lexeme);
		
		// Get next token and check that it is an equal sign.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != eqsym)
			return 2;
		
		// Get the next token and check that it is a number.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != numbersym)
			return 1;
		// Update the symbol's value.
		newSym.value = atoi(getCurrentToken().lexeme);
		
		// Add the new symbol to the table.
		addSymbol(&symbolTable, newSym);
		
		// Get next token.
		printCurrentToken();
		nextToken();
	} while(getCurrentTokenType() == commasym);
	
	// Check for semicolon and get the next token.
	if(getCurrentTokenType() != semicolonsym)
		return 5;
	printCurrentToken();
	nextToken();

    // Successful parsing.
    return 0;
}

int var_declaration()
{
    do
	{
		// Declare a new symbol and set its type and level
		// values.
		Symbol newSym;
		newSym.type = VAR;
		newSym.level = currentLevel;
		
		// Get next token and check that it is an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;
		// Update the symbol's name.
		strcpy(newSym.name, getCurrentToken().lexeme);
		
		// Get the next token.
		printCurrentToken();
		nextToken();
		
		// Add the new symbol to the table.
		addSymbol(&symbolTable, newSym);
	} while(getCurrentTokenType() == commasym);
	
	// Check for semicolon and get the next token.
	if(getCurrentTokenType() != semicolonsym)
		return 4;
	printCurrentToken();
	nextToken();

    return 0;
}

int proc_declaration()
{
	// Error variable for tracking error codes.
	int err = 0;
	
	// While loop parses procedure declaration.
    while(getCurrentTokenType() == procsym)
	{
		// Declare a new symbol and set its type and level
		// values.
		Symbol newSym;
		newSym.type = PROC;
		newSym.level = currentLevel;
		
		// Get next token and check that it is an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;
		// Update the symbol's name.
		strcpy(newSym.name, getCurrentToken().lexeme);
		
		// Add the new symbol to the table.
		addSymbol(&symbolTable, newSym);
		
		// Get next token and check that it is a semicolon.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != semicolonsym)
			return 5;
		
		// Get next token.
		printCurrentToken();
		nextToken();
		
		// Increment the current level for the next block and
		// decrement it after the block is finished.
		currentLevel++;
		err = block();
		currentLevel--;
		
		// If error is found return immediately.
		if(err != 0)
			return err;
		
		// Check for semicolon after new block.
		if(getCurrentTokenType() != semicolonsym)
			return 5;
		
		// Get next token.
		printCurrentToken();
		nextToken();
	}

    return err;
}

int statement()
{
    printNonTerminal(STATEMENT);
	
	// Error variable for tracking error codes.
	int err = 0;
	
	// Statement that begins with an identifier symbol.
    if(getCurrentTokenType() == identsym)
	{
		// Get next token and check if it is a become symbol.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != becomessym)
			return 7;
		
		// Get next token and pass to expression.
		printCurrentToken();
		nextToken();
		err = expression();
	}
	// Statement that begins with a call symbol.
	else if(getCurrentTokenType() == callsym)
	{
		// Get next token and check if it is an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 8;
		
		// Get next token.
		printCurrentToken();
		nextToken();
	}
	// Statement that begins with begin symbol.
	else if(getCurrentTokenType() == beginsym)
	{
		// Get next token and pass to statement.
		printCurrentToken();
		nextToken();
		err = statement();
		
		while (getCurrentTokenType() == semicolonsym)
		{
			// Get next token and pass to statement.
			printCurrentToken();
			nextToken();
			err = statement();
		}
		
		// Check for end symbol and get the next token.
		if(getCurrentTokenType() != endsym)
			return 10;
		printCurrentToken();
		nextToken();
	}
	// Statement that begins with if symbol.
	else if(getCurrentTokenType() == ifsym)
	{
		// Get next token and pass to condition.
		printCurrentToken();
		nextToken();
		err = condition();
		
		// Check the token is a then symbol.
		if(getCurrentTokenType() != thensym)
			return 9;
		
		// Get next token and pass to statement.
		printCurrentToken();
		nextToken();
		err = statement();
		
		// Check for else statement. Get the next token and pass
		// to statement if an else token is the current token.
		if(getCurrentTokenType() == elsesym)
		{
			printCurrentToken();
			nextToken();
			err = statement();
		}
	}
	// Statement that begins with while symbol.
	else if(getCurrentTokenType() == whilesym)
	{
		// Get next token and pass to condition.
		printCurrentToken();
		nextToken();
		err = condition();
		
		// Check the token is a do symbol.
		if(getCurrentTokenType() != dosym)
			return 11;
		
		// Get next token and pass to statement.
		printCurrentToken();
		nextToken();
		err = statement();
	}
	// Statement that begins with write symbol.
	else if(getCurrentTokenType() == writesym)
	{
		// Get next token and check if its an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;
		
		// Get next token.
		printCurrentToken();
		nextToken();
	}
	// Statement that begins with read symbol.
	else if(getCurrentTokenType() == readsym)
	{
		// Get next token and check if its an identifier.
		printCurrentToken();
		nextToken();
		if(getCurrentTokenType() != identsym)
			return 3;
		
		// Get next token.
		printCurrentToken();
		nextToken();
	}

    return err;
}

int condition()
{
    printNonTerminal(CONDITION);
	
	// Error variable for tracking errors.
	int err = 0;
	
	// Check if the condition begins with an odd symbol.
    if(getCurrentTokenType() == oddsym)
	{
		// Get next token and pass to expression.
		printCurrentToken();
		nextToken();
		err = expression();
	}
	else
	{
		err = expression();
		
		// Check if the current token is a relation symbol.
		if(getCurrentTokenType() != relop())
			return 12;
		
		// Get next token and pass to expression.
		printCurrentToken();
		nextToken();
		err = expression();
	}

    return err;
}

int relop()
{
    printNonTerminal(REL_OP);

	// Compare the current token with all of the relation ops.
    if(getCurrentTokenType() == eqsym || 
	   getCurrentTokenType() == neqsym ||
	   getCurrentTokenType() == lessym ||
	   getCurrentTokenType() == leqsym ||
	   getCurrentTokenType() == gtrsym ||
	   getCurrentTokenType() == geqsym)
	   return getCurrentTokenType();
	
	// Failure to find relation operator.
    return 0;
}

int expression()
{
    printNonTerminal(EXPRESSION);

	// Error variable for tracking error codes.
	int err = 0;
	
	// Get the next token if the current is a plus or minus sign.
    if(getCurrentTokenType() == plussym || 
	   getCurrentTokenType() == minussym)
	{
		printCurrentToken();
		nextToken();
	}
	
	err = term();
	
	// Continue parsing until the end of the expression.
	while(getCurrentTokenType() == plussym || 
	      getCurrentTokenType() == minussym)
	{
		printCurrentToken();
		nextToken();
		err = term();
	}

    return err;
}

int term()
{
    printNonTerminal(TERM);

	// Error variable for tracking errors.
	int err = 0;
	
    err = factor();
	
	// Continue parsing until the end of the term expression.
	while(getCurrentTokenType() == multsym || 
	      getCurrentTokenType() == slashsym)
	{
		printCurrentToken();
		nextToken();
		err = factor();
	}

    return 0;
}

/**
 * The below function is left fully-implemented as a hint.
 * */
int factor()
{
    printNonTerminal(FACTOR);

    /**
     * There are three possibilities for factor:
     * 1) ident
     * 2) number
     * 3) '(' expression ')'
     * */

    // Is the current token a identsym?
    if(getCurrentTokenType() == identsym)
    {
        // Consume identsym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Success
        return 0;
    }
    // Is that a numbersym?
    else if(getCurrentTokenType() == numbersym)
    {
        // Consume numbersym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Success
        return 0;
    }
    // Is that a lparentsym?
    else if(getCurrentTokenType() == lparentsym)
    {
        // Consume lparentsym
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..

        // Continue by parsing expression.
        int err = expression();

        /**
         * If parsing of expression was not successful, immediately stop parsing
         * and propagate the same error code by returning it.
         * */
        
        if(err) return err;

        // After expression, right-parenthesis should come
        if(getCurrentTokenType() != rparentsym)
        {
            /**
             * Error code 13: Right parenthesis missing.
             * Stop parsing and return error code 13.
             * */
            return 13;
        }

        // It was a rparentsym. Consume rparentsym.
        printCurrentToken(); // Printing the token is essential!
        nextToken(); // Go to the next token..
    }
    else
    {
        /**
          * Error code 24: The preceding factor cannot begin with this symbol.
          * Stop parsing and return error code 24.
          * */
        return 14;
    }

    return 0;
}
