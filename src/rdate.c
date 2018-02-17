#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#if 0
TODO --

Add times, not just dates.
#endif

#define VERSION "0.1.0"
#define ROMAN_YEAR_OFFSET 753
#define NUMSTR_BUFLEN 12
#define FULLSTR_BUFLEN 255

#define DIVIDES(x, y) (!((x) % (y)))

#ifdef __STDC_VERSION__
#if __STDC_VERSION__ >= 199901L
#define INLINE inline
#endif
#endif
#ifndef INLINE
#define INLINE
#endif

/****************************************************************************/
/* String constants                                                         */
/****************************************************************************/

static const char *const month_short[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static const char *const month_long[12] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

static const char *const day_short[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};

static const char *const day_long[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static const char special_char[3] = {
	'K', 'N', 'I'
};

static const char* const special_str[3] = {
	"Kalends", "Nones", "Ides"
};

/****************************************************************************/
/* Globals                                                                  */
/****************************************************************************/
static bool do_dozenal = false;

/****************************************************************************/
/* Structs                                                                  */
/****************************************************************************/
enum SpecialDay {
	Kalends,
	Nones,
	Ides
};

struct RDate {
	enum SpecialDay day;
	int effective_year;
	char dow;
	char days_to;
	char month;
	char hour;
	char min;
	char sec;
};

/****************************************************************************/
/* Inline Functions                                                         */
/****************************************************************************/

static INLINE int adj_year(int year)
{
	return year + 1900 + ROMAN_YEAR_OFFSET;
}

static INLINE const char *const ending(int i)
{
	if (i / 10 == 1)
		return "th";
	if (i % 10 == 1)
		return "st";
	if (i % 10 == 2)
		return "nd";
	if (i % 10 == 3)
		return "rd";
	return "th";
}

static INLINE bool is_leap_year(int year)
{
	return DIVIDES(year, 4) && (DIVIDES(year, 400) || !DIVIDES(year, 100));
}

static INLINE int day_of_ides(int month_num)
{
	switch (month_num) {
		case 2:
		case 4:
		case 6:
		case 9:
			return 15;
		default:
			return 13;
	}
}

static INLINE int day_of_nones(int month_num)
{
	return day_of_ides(month_num) - 8;
}

/****************************************************************************/
/* Full functions                                                           */
/****************************************************************************/

static int days_in_month(int month_num, int year)
{
	switch (month_num) {
		case 8:
		case 3:
		case 5:
		case 10:
			return 30;
		case 1:
			return is_leap_year(year) ? 29 : 28;
		default:
			return 31;
	}
}

static int ipow(int x, int y)
{
	int r = 1;
	while (y --> 0)
		r *= x;
	return r;
}

/****************************************************************************/
/* char *num_to_string(int innun, int at_least) {{{                         */
/****************************************************************************/
static char *num_to_string(int innum, int at_least)
{
	char *ret = malloc(NUMSTR_BUFLEN);
	char *adjbuf;
	int num[NUMSTR_BUFLEN];
	int curdex = 0;
	int outdex = 0;
	int curnum = 0;
	char thischar;
	const int base = do_dozenal ? 12 : 10;

	memset(ret, 0, NUMSTR_BUFLEN);
	memset(num, 0, NUMSTR_BUFLEN * sizeof(int));

	while (innum > 0) {
		if (!(innum % ipow(base, curdex + 1))) {
			num[curdex++] = curnum;
			curnum = 0;
		} else {
			curnum++;
			innum -= ipow(base, curdex);
		}
	}

	/* Final power */
	num[curdex] = curnum;

	/* Convert to string */
	curdex = NUMSTR_BUFLEN;
	while (curdex --> 0) {
		switch (num[curdex]) {
			case 10:
				thischar = 'D';
				break;
			case 11:
				thischar = 'E';
				break;
			case 0:
				if (!outdex)
					thischar = '\0';
				else
			default:
				thischar = '0' + num[curdex];
		}

		if (thischar)
			ret[outdex++] = thischar;
	}

	/************************************************/
	/* Check we have enough chars                   */
	/************************************************/
	if (outdex < at_least) {
		adjbuf = malloc(at_least + 1);
		for (thischar = 0; thischar < at_least - outdex; thischar++) {
			adjbuf[thischar] = '0';
		}
		strcpy(adjbuf + at_least - outdex, ret);
		free(ret);
		ret = adjbuf;
		adjbuf = NULL;
	}

	return ret;
}
/****************************************************************************/
/* }}}                                                                      */
/****************************************************************************/

static INLINE const char *const month_name(int month_num)
{
	if (month_num < 0 || month_num >= 12)
		return "Unknown";

	return month_long[month_num];
}

/****************************************************************************/
/* struct RDate get_date() {{{                                              */
/****************************************************************************/
static struct RDate get_date()
{
	struct tm *curtime;
	struct RDate ret;
	const time_t timeval = time(NULL);
	int nones, ides;

	/************************************************/
	/* Get sys time                                 */
	/************************************************/
	curtime = gmtime(&timeval);

	ret.effective_year = curtime->tm_year;
	ret.month = curtime->tm_mon;
	ret.days_to = 0;
	ret.dow = curtime->tm_wday;
	ret.hour = curtime->tm_hour;
	ret.min = curtime->tm_min;
	ret.sec = curtime->tm_sec;

	if ((curtime->tm_mon <= 1)
	    || ((curtime->tm_mon == 2)
	        && (curtime->tm_mday > day_of_nones(curtime->tm_mon))))
		ret.effective_year--;

	nones = day_of_nones(curtime->tm_mon);
	ides = day_of_ides(curtime->tm_mon);

	if (curtime->tm_mday == 1)
		ret.day = Kalends;
	else if (curtime->tm_mday == nones)
		ret.day = Nones;
	else if (curtime->tm_mday == ides)
		ret.day = Ides;
	else if (curtime->tm_mday < nones) {
		ret.day = Nones;
		ret.days_to = nones - curtime->tm_mday;
	} else if (curtime->tm_mday < ides) {
		ret.day = Ides;
		ret.days_to = ides - curtime->tm_mday;
	} else {
		ret.day = Kalends;
		ret.days_to = days_in_month(curtime->tm_mon, curtime->tm_year) + 1 - curtime->tm_mday;
		ret.month = (ret.month + 1) % 12;
	}

	return ret;
}
/****************************************************************************/
/* }}}                                                                      */
/****************************************************************************/

/****************************************************************************/
/* char *date_str_fmt(const char *const fmt_str, struct RDate *date) {{{    */
/****************************************************************************/
static char *date_str_fmt(const char *const fmt_str, struct RDate *date)
{
	char *ret = malloc(FULLSTR_BUFLEN);
	char *retdex = ret;
	char *numstr = NULL;
	const char *fmtdex;
	bool perc = false;

	for (fmtdex = fmt_str; *fmtdex; fmtdex++) {
		if (!perc) {
			if (*fmtdex == '%')
				perc = true;
			else
				*retdex++ = *fmtdex;
			continue;
		}

		/************************************************/
		/* We're about to handle this...                */
		/************************************************/
		perc = false;

#define PRINT(word) retdex += sprintf(retdex, "%s", (word))
#define PRINTF(pat, word) retdex += sprintf(retdex, (pat), (word))
		switch (*fmtdex) {
			case '%':
				*retdex++ = '%';
				break;
			case 'n':
				*retdex++ = '\n';
				break;
			case 't':
				*retdex++ = '\t';
				break;
			case 'a':
				PRINT(day_short[date->dow]);
				break;
			case 'A':
				PRINT(day_long[date->dow]);
				break;
			case 'b':
			case 'h':
				PRINT(month_short[date->month]);
				break;
			case 'B':
				PRINT(month_long[date->month]);
				break;
			case 'd':
				if (date->days_to) {
					numstr = num_to_string(date->days_to, 2);
					retdex += sprintf(
						retdex,
						"%s%c",
						numstr,
						special_char[date->day]);
				} else
					PRINTF("%c", special_char[date->day]);
				break;
			case 'D':
				if (date->days_to) {
					numstr = num_to_string(date->days_to, 2);
					retdex += sprintf(
						retdex,
						"%s %s",
						numstr,
						special_str[date->day]);
				} else
					PRINT(special_str[date->day]);
				break;
			case 'e':
				if (date->days_to) {
					numstr = num_to_string(date->days_to, 0);
					retdex += sprintf(
						retdex,
						"%s%c",
						numstr,
						special_char[date->day]);
				} else
					PRINTF("%c", special_char[date->day]);
				break;
			case 'E':
				if (date->days_to) {
					numstr = num_to_string(date->days_to, 0);
					retdex += sprintf(
						retdex,
						"%s %s",
						numstr,
						special_str[date->day]);
				} else
					PRINTF("%s", special_str[date->day]);
				break;
			case 'm':
				PRINTF("%.2d", date->month + 1);
				break;
			case 'u':
				PRINTF("%d", date->dow ? date->dow : 7);
				break;
			case 'w':
				PRINTF("%d", date->dow);
				break;
			case 'y':
				numstr = num_to_string(adj_year(date->effective_year), 2);
				PRINT(numstr + strlen(numstr) - 2);
				break;
			case 'Y':
				numstr = num_to_string(adj_year(date->effective_year), 4);
				PRINT(numstr);
				break;
			default:
				fprintf(stderr, "Unknown format char %c\n", *fmtdex);
				free(ret);
				return NULL;
		}

		if (numstr) {
			free(numstr);
			numstr = NULL;
		}
#undef PRINT
#undef PRINTF
	}

	*retdex = '\0';
	return ret;
}
/****************************************************************************/
/* }}}                                                                      */
/****************************************************************************/

int main(int argc, char **argv)
{
	struct RDate date;
	char *time_str = NULL;
	char *fmt_str = NULL;
	int ii;
	char ret = 0;
	char *progname, *p;

	srandom(time(NULL));
	progname = argv[0];
	if ((p = strrchr(progname, '/')) != NULL)
		progname = p + 1;

	/************************************************/
	/* Argparse                                     */
	/************************************************/
	for (ii = 1; ii < argc; ii++) {
		switch(argv[ii][0]) {
			case '+':
				fmt_str = argv[ii]+1;
				break;
			case '-':
				switch (argv[ii][1]) {
					case 'd':
						do_dozenal = true;
						break;
					case 'D':
						do_dozenal = false;
						break;
					case 'V':
						/* Do version */
						printf("%s (%s)\n", progname, VERSION);
						goto CLEANUP;
					case 'h':
						/* Do help */
						printf("usage: %s [options] [+format]\n", progname);
						goto CLEANUP;
					default:
						/* Do usage */
						fprintf(stderr, "usage: %s [options] [+format]\n", progname);
						ret = 1;
						goto CLEANUP;
				}
				break;
			default:
				fprintf(stderr, "Unknown arg %s\n", argv[ii]);
				ret = 1;
				goto CLEANUP;
		}
	}

	if (!fmt_str)
		fmt_str = "%A, %D %B, %Y";

	/************************************************/
	/* Prepare time                                 */
	/************************************************/
	date = get_date();
	time_str = date_str_fmt(fmt_str, &date);

	if (time_str)
		printf("%s\n", time_str);

CLEANUP:

	if (time_str)
		free(time_str);

	return ret;
}

