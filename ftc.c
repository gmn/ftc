//--------------------
//
// ftc.c - floating point two argument calculator.  short and sweet,
// mostly for doing the many 2 argument calculations that I need for
// scripts or just whatever -- GMN
//
//--------------------
#include <stdio.h>
#include <stdlib.h> //atof
#include <string.h> //strchr
#include <math.h>

// 3rd field is svn number
// 1st & 2nd numbers are a fraction indicating how finished I feel the 
//  program is.  "1.0" being a decent stable release.
#define FTC_VERSION "0.1.9"

char program_name[256];

typedef enum {
        OP_NONE,
        OP_MULTIPLY,
        OP_DIVIDE,    
        OP_ADD,
        OP_MODULUS,
        OP_SUBTRACT,
        OP_RAISE_TO_THE_POWER,
        OP_NATURAL_LOG,
        OP_LOG_BASE10,
        OP_LOG_ARBITRARY_BASE,
} operator_t;

enum output_t {
	OUTPUT_NORMAL,
	OUTPUT_BINARY,
	OUTPUT_HEXADECIMAL
};

void saveArgf( float f ) {
    char buf[128]; 
#ifdef _WIN32
    sprintf(buf, "echo %f > /tmp/.ftc_last_result", f);
#else
    snprintf(buf, 128, "echo %f > /tmp/.ftc_last_result", f);
#endif
    system(buf);
}

void saveArgi( int i ) {
    char buf[128]; 
#ifdef _WIN32
    sprintf(buf, "echo %i > /tmp/.ftc_last_result", i);
#else
    snprintf(buf, 128, "echo %i > /tmp/.ftc_last_result", i);
#endif
    system(buf);
}

/*
void saveArgf( float f ) {
    FILE *fp = fopen("/tmp/.ftc_last_result", "w+");
    if ( !fp )
        return;
    fprintf( fp, "%f", f );
    fclose( fp );
}

void saveArgi( int i ) {
    FILE *fp = fopen("/tmp/.ftc_last_result", "w+");
    if ( !fp )
        return;
    fprintf( fp, "%i", i );
    fclose( fp );
}
*/

char * trim ( char *val ) {
	char buf[256];
	char *p = buf;
	char *v = val;

	// first skip leading spaces
	while ( *v == ' ' )
		++v;
	
	// copy any valid thing thats not a space
	while ( *v && *v != ' ' ) {
		*p++ = *v++;
	}
	*p = '\0';	// seal the end
	p = buf; v = val;
	// copy back
	while ( *v++ = *p++ )
		;
	return val;
}

char *getend( char *buf, int lim ) {
    if ( !buf )
        return NULL;
    while ( *buf && lim > 0 ) {
        ++buf;
        --lim;
    }
    if ( lim >= 0 )
        return buf;
    return NULL;
}

int checkStoredResult( char *str, char *tok ) {
    char res[128];
    char buf[1024];
    char *e;
    int n, i;
    
    FILE *fp = fopen( "/tmp/.ftc_last_result", "r" );
    if ( !fp )
        return 0;

    n = fread( res, 1, 128, fp );
    fclose( fp );
    res[n] = '\0';
	for (i = 0; i < n; i++ ) {
		if ( res[i] == '\n' ) { res[i] = '\0'; break; }
	}
    
    strncpy( buf, str, 1024);	// copy rest of str to buf
printf( "buf: %s, str: %s\n", buf, str );
    strncpy( str, tok, 256 );	// copy current tok back over str
printf( "str: %s, tok: %s\n", str, tok );
    strncpy( tok, res, 128 );   // strncpy pads end of tok w/ \0\0\0...
printf( "tok: %s, res: %s\n", tok, res );
    e = getend( str, 256 );
    n = (int)(e - str);
    *e++ = ' ';
    //strncpy( e, buf, (int)(e-str) );	// put old str bck on end
    strcpy( e, buf );	// put old str bck on end
    return 1;
}

int isOperator( char *t ) {
	char c = 0;
    if ( !t )
        return 0;
    switch ( *t ) {
	case '+': case '-': // check for sign-extended numbers
		if ( (c=*(t+1)) && c >= '0' && c <= '9' )
			return 0;
    case '*': case '/': case '%': case '^':
        return 1;
    }
    return 0;
}

int hexLetter( int h ) {
    if ( h >= '0' && h <= '9' ) {
        return h - 48;
    } else if ( h >= 'A' && h <= 'F' ) {
    	return h - 55;
	} else if ( h >= 'a' && h <= 'f' ) {
		return h - 87;
	}
	return 0;
}

int hexToInt( char *hexstr ) {
    char *p = hexstr, *e;
    int val = 0;
    int mult = 1;

    if ( *p++ != '0' ) 
		return 0;

	if ( *p != 'x' && *p != 'h' ) 
        return 0;
	++p;

    e = p + strlen( p );
    
    while ( e != p ) {
        val += hexLetter(*--e) * mult;
        mult *= 16;
    }

    return val;
}

int binToInt( char *binstr ) {
    char *p = binstr, *e;
    int val = 0;
    int mult = 1;

    if ( *p++ != '0' || *p++ != 'b' )
        return 0;

    e = p + strlen( p );
    
    while ( e != p ) {
        if ( *--e == '1' ) {
            val += mult;
        }
        mult *= 2;
    }
    return val;
}

int octToInt( char *octstr ) {
	char *p = octstr, *e;
	int mult = 1;
	int val = 0;
	if ( *p++ != '0' )
		return 0;
	e = p + strlen( p );
	while ( e-- != p ) {
    	if ( *e >= '0' && *e <= '7' ) {
        	val += ( *e - 48 ) * mult;
		}
		mult *= 8;
	}
	return val;
}

// fills buf with binary representation of the integer
char * intToBin( char *buf, int in ) {

	// find highest bit, so we know how far to shift output
	int move = 0;
	unsigned int test = 1 << 31;
	int spaces = 0;
	unsigned int shift = 0;

	while ( test > 0 ) {
		if ( test <= (unsigned)in )
			break;
		test >>= 1;
		++move;
	}
	// if it's negative, show all bits
	if ( move >= 32 )
		move = 0;

	if ( 32 - move >= 8 ) {
		++spaces;
	}
	if ( 32 - move >= 16 ) {
		++spaces;
	}
	if ( 32 - move >= 24 ) {
		++spaces;
	}

	buf = buf + 32 + 2 - move + spaces;
	*buf-- = '\0';
	while ( shift < 32 - move ) {
		if ( (1 << shift) & in ) {
			*buf = '1';
		} else {
			*buf = '0';
		}
		--buf;
		++shift;

		if ( shift == 8 || shift == 16 || shift == 24 ) {
			if ( shift != 32 - move )
				*buf-- = ' ';
		}
	}
	*buf-- = 'b';
	*buf = '0';
	return buf;
}

char * clipLeadingWord(char *string, char *word);
int yankCharacter(char *str, const char y);
int isEquallyDivisible(int, int );

void usage( const char *name ) {
    fprintf(stderr, "usage: %s \'number [-H|-B] (*|/|+|-|%%) number\'\n", name ) ;
}

// lays a str into buf, and returns a pointer into buf at the char that
//  immediately follows the one last laid into it
char *laystr( char *buf, const char *s )
{
    if ( !s )
        return buf;
    if ( !buf ) 
        return NULL;
    while ( (*buf = *s++) ) 
        ++buf;
    return buf;        
}

int main (int argc, char *argv[])
{
    int floatState = 0;
    float arg1f, arg2f;
    int arg1i, arg2i;
    operator_t op = OP_NONE;
    char string[256];
    char token[256];
    char args[16384];
    char *p;
	enum output_t out_type = OUTPUT_NORMAL;

    // zero out buffers
    memset( string,         0, sizeof(string) );
    memset( token,            0, sizeof(token) );
    memset( program_name,   0, sizeof(program_name) );
    memset( args,           0, sizeof(args) );

    // get program name
    if ( (p = strrchr( argv[0], '/' )) ) 
        strncpy( program_name, ++p, sizeof(program_name) );
    else
        strncpy( program_name, argv[0], sizeof(program_name) );

	// alias
	p = argv[1];

    // check for usage, print and exit sort of thing
    if ( argc == 3 ) {
		// hex output
		if ( !strcmp( "-H", argv[1] ) ) {
			out_type = OUTPUT_HEXADECIMAL;
		}
		// binary output
		else if ( !strcmp( "-B", argv[1] ) ) {
			out_type = OUTPUT_BINARY;
		} 
		// mystery argument
		else {
        	usage( program_name );
        	return 1;
		}
		p = argv[2];
	} else if ( argc != 2 ) {
        usage( program_name );
        return 1;
    }

	if ( !strcmp( argv[1], "--version" ) ) {
		fprintf( stdout, "ftc Version: %s (c) 2008, Greg Naughton\n", FTC_VERSION );
		return 0;
	}
	if ( !strcmp( argv[1], "--help" ) ) {
		usage( program_name );
		return 0;
	}

    // copy math string
    strcpy( string, p );

    // first argument
    clipLeadingWord(string, token);

    if ( isOperator(token) ) {
printf( "isOperator, string: %s, token: %s \n", string, token );
        if (!checkStoredResult(string, token)) {
printf( "!isStoredResult, string: %s, token: %s\n", string, token );
            fprintf(stderr, "error: argument not found\n");
            exit(-1);
        }
else
printf( "isStoredResult, string: \"%s\", token: \"%s\"\n", string, token );
    }

    if ( strstr( token, "log" ) ) {
        op = OP_LOG_BASE10;
        clipLeadingWord(string, token);
        arg2f = atof(token);
    } else if ( strstr( token, "ln" ) ) {
        op = OP_NATURAL_LOG;
        clipLeadingWord(string, token);
        arg2f = atof(token);
    } else if ( strchr(token, '.') ) {     
        arg1f = atof(token);
        floatState = 1;
    } else if ( strchr(token, 'x') || strchr(token, 'h') ) {
        arg1i = hexToInt( token );
    } else if ( strchr( token , 'b' ) ) {
        arg1i = binToInt( token );
	} else if ( *trim(token) == '0' ) {
		arg1i = octToInt( token );
    } else 
        arg1i = atoi(token);

    // if log, evaluate and return
    if ( op == OP_NATURAL_LOG ) {
        arg1f = logf( arg2f );
        fprintf(stdout, "%f\n", arg1f);
        saveArgf( arg1f );
        return 0;
    } else if ( op == OP_LOG_BASE10 ) {
        arg1f = log10f( arg2f );
        fprintf(stdout, "%f\n", arg1f);
        saveArgf( arg1f );
        return 0;
    }

    // hopefully operator
    clipLeadingWord(string, token);
    if (strchr(token, '*') != NULL) 
        op = OP_MULTIPLY;
    else if (strchr(token, '/') != NULL) 
        op = OP_DIVIDE;
    else if (strchr(token, '-') != NULL)
        op = OP_SUBTRACT;
    else if (strchr(token, '+') != NULL)
        op = OP_ADD;
    else if (strchr(token, '%') != NULL)
        op = OP_MODULUS;
    else if (strchr(token, '^') != NULL)
        op = OP_RAISE_TO_THE_POWER;
    else {
        // no operator, print the first argument and exit
        if (floatState > 0) {
            fprintf(stdout, "%f\n", arg1f);
            saveArgf( arg1f );
        } else {
			if ( OUTPUT_BINARY == out_type ) {
	        	fprintf(stdout, "%s\n", intToBin(token, arg1i) );
			} else if ( OUTPUT_HEXADECIMAL == out_type ) {
	        	fprintf(stdout, "0x%X\n", arg1i);
			} else {
	        	fprintf(stdout, "%d\n", arg1i);
			}
            saveArgi( arg1i );
        }
        return 0;
    }

    // second argument
    clipLeadingWord( string, token );
    if (strchr(token, '.') )  {     // || strchr(token, 'x') ) {
        arg2f = atof(token);
        floatState += 2;
    } else if ( strchr( token , 'x' ) || strchr(token, 'h') ) {
        arg2i = hexToInt( token );
    } else if ( strchr( token , 'b' ) ) {
        arg2i = binToInt( token );
	} else if ( *trim(token) == '0' ) {
		arg2i = octToInt( token );
    } else
        arg2i = atoi(token);

    // setup second arg
    switch (floatState) {
    case 1: 
        arg2f = arg2i; 
        break;
    case 2: 
        arg1f = arg1i; 
        break;
    }


    // Find out if divisor is equally divisible by dividend or whatever
    // if it isn't, promote the whole mess to float, if it is, 
    //  then dont worry about it.
    if (!floatState && op == OP_DIVIDE) {
        if (!isEquallyDivisible(arg1i, arg2i)) {
            ++floatState;
            arg1f = arg1i;
            arg2f = arg2i;
        }
    }

    switch (op) {
    case OP_MULTIPLY:
        if (floatState > 0)
            arg1f *= arg2f;
        else 
            arg1i *= arg2i;
        break;
    case OP_DIVIDE:
        if (floatState > 0)
            arg1f /= arg2f;
        else 
            arg1i /= arg2i;
        break;
    case OP_SUBTRACT:
        if (floatState > 0)
            arg1f -= arg2f;
        else 
            arg1i -= arg2i;
        break;
    case OP_ADD:
        if (floatState > 0)
            arg1f += arg2f;
        else 
            arg1i += arg2i;
        break;
    case OP_MODULUS:
        if (floatState > 0) {
            fprintf(stderr, "error: cant take modulus of floats!\n");
            exit(1);
        } else
            arg1i %= arg2i;
           break;
    case OP_RAISE_TO_THE_POWER:
        if ( floatState > 0.0f ) {
            arg1f = powf( arg1f, arg2f );
        } else {
            arg1i = (int) powf( (float)arg1i, (float)arg2i );
        }
    default:
        break;
    }

    if (floatState > 0) {
        fprintf(stdout, "%f\n", arg1f);
        saveArgf( arg1f );
    } else {
		if ( OUTPUT_BINARY == out_type ) {
        	fprintf(stdout, "%s\n", intToBin(token, arg1i) );
		} else if ( OUTPUT_HEXADECIMAL == out_type ) {
        	fprintf(stdout, "0x%X\n", arg1i);
		} else {
        	fprintf(stdout, "%d\n", arg1i);
		}
        saveArgi( arg1i );
    }

    return 0;
}

// leading word in string is clipped off to the space,
// stored in word and returns pointer to word
char *clipLeadingWord(char *string, char *word)
{
    int i,j;
    char buf[256];
    memset(buf, 0, sizeof(buf));

    i = j = 0;
    
    // move past leading space
    while (string[j] == ' ' || string[j] == '\t')
        ++j;

    while ((word[i] = string[j]) != ' ' && word[i] != '\0')
        ++i, ++j;

    if (word[i] != '\0')
        word[i++] = '\0';

    strncpy(buf, &string[j], 256);
    memset(string, 0, 256);
    strncpy(string, buf, 256);

    return word;
}

// yank y from first occurrence in str, replace with space
// 
// returns 1 success, 0 failure
int yankCharacter(char *str, const char y)
{
    int i;

    while (str[i] != '\0') 
    {
        if (str[i] == y) 
        {
            str[i] = ' ';
            return 1;
        } 
        ++i;
    }
    return 0;
}

int isEquallyDivisible(int a, int b)
{
    float sum = (float) a / (float) b;
    int sumi = (int) sum;

    sum -= sumi;

    if (sum == 0.0) 
        return 1;

    return 0;
}
