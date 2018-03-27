#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <string.h>

#define VERSION "0.1.0"
#define ROMAN_YEAR_OFFSET 753
#define NUMSTR_BUFLEN 12
#define FULLSTR_BUFLEN 255

#define DIVIDES(x, y) (!((x) % (y)))
#define FREE(x) do { if (x) { free(x); x = NULL; } } while (0)
#define MOVE(x, y) do { x = y; y = NULL; } while (0)

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

static const char *const month[12] = {
	"January", "February", "March", "April", "May", "June",
	"July", "August", "September", "October", "November", "December"
};

static const char *const day[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday",
	"Thursday", "Friday", "Saturday"
};

static const char* const special_str[3] = {
	"Kalends", "Nones", "Ides"
};

/****************************************************************************/
/* Globals                                                                  */
/****************************************************************************/
static bool do_dozenal = false;
static char *progname = NULL;

/****************************************************************************/
/* Structs                                                                  */
/****************************************************************************/
enum SpecialDay {
	Kalends,
	Nones,
	Ides
};

enum ArgCode {
	Aok,
	Quit,
	Fail
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
/* Prototypes                                                               */
/****************************************************************************/
static INLINE int day_of_nones(int month_num);
static INLINE int day_of_ides(int month_num);
static INLINE bool is_leap_year(int year);
static INLINE const char *const ending(int i);
static INLINE int adj_year(int year);
static char *numstrn(int innum, int at_least);
static char *numstr(int i);
static char *date_str_fmt(const char *const fmt_str, struct RDate *date);
static int ipow(int x, int y);
static int days_in_month(int month_num, int year);
static struct RDate get_date();
static enum ArgCode short_arg(char argl);
static enum ArgCode short_args(char *argl);

/****************************************************************************/
/* Inline Functions {{{                                                     */
/****************************************************************************/

static INLINE int adj_year(int year)
{
	return year + 1900 + ROMAN_YEAR_OFFSET;
}

static INLINE const char *const ending(int i)
{
	if (i / 10 == 1) return "th";
	if (i % 10 == 1) return "st";
	if (i % 10 == 2) return "nd";
	if (i % 10 == 3) return "rd";
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
/* }}}                                                                      */
/****************************************************************************/

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
	while (y) {
		if (y & 1)
			r *= x;
		y >>= 1;
		x *= x;
	}
	return r;
}

static char *numstr(int i)
{
	char *ret = numstrn(i, 0);
	return ret;
}

/****************************************************************************/
/* char *numstrn(int innun, int at_least) {{{                               */
/****************************************************************************/
static char *numstrn(int innum, int at_least)
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
		FREE(ret);
		MOVE(ret, adjbuf);
	}

	return ret;
}
/****************************************************************************/
/* }}}                                                                      */
/****************************************************************************/

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
	char *prtstr = NULL;
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
			case 'a':
				PRINTF("%.3s", day[date->dow]);
				break;
			case 'A':
				PRINT(day[date->dow]);
				break;
			case 'b':
			case 'h':
				PRINTF("%.3s", month[date->month]);
				break;
			case 'B':
				PRINT(month[date->month]);
				break;
			case 'd':
				prtstr = numstrn(date->days_to, 2);
				retdex += sprintf(
					retdex,
					"%s%c",
					prtstr,
					*(special_str[date->day]));
				break;
			case 'D':
				prtstr = numstrn(date->days_to, 2);
				retdex += sprintf(
					retdex,
					"%s %s",
					prtstr,
					special_str[date->day]);
				break;
			case 'e':
				if (date->days_to) {
					prtstr = numstr(date->days_to);
					retdex += sprintf(
						retdex,
						"%s%c",
						prtstr,
						*(special_str[date->day]));
				} else
					PRINTF("%c", *(special_str[date->day]));
				break;
			case 'E':
				if (date->days_to) {
					prtstr = numstr(date->days_to);
					retdex += sprintf(
						retdex,
						"%s %s",
						prtstr,
						special_str[date->day]);
				} else
					PRINT(special_str[date->day]);
				break;
			case 'H':
				prtstr = numstr(date->hour);
				PRINT(prtstr);
				break;
			case 'I':
				prtstr = numstr(date->hour % 12 + 1);
				PRINT(prtstr);
				break;
			case 'm':
				prtstr = numstrn(date->month + 1, 2);
				PRINT(prtstr);
				break;
			case 'M':
				prtstr = numstrn(date->min, 2);
				PRINT(prtstr);
				break;
			case 'n':
				*retdex++ = '\n';
				break;
			case 'S':
				prtstr = numstrn(date->sec, 2);
				PRINT(prtstr);
				break;
			case 't':
				*retdex++ = '\t';
				break;
			case 'u':
				prtstr = numstr(date->dow ? date->dow : 7);
				PRINT(prtstr);
				break;
			case 'w':
				prtstr = numstr(date->dow);
				PRINT(prtstr);
				break;
			case 'y':
				prtstr = numstrn(adj_year(date->effective_year), 2);
				PRINT(prtstr + strlen(prtstr) - 2);
				break;
			case 'Y':
				prtstr = numstrn(adj_year(date->effective_year), 4);
				PRINT(prtstr);
				break;
			default:
				fprintf(stderr, "Unknown format char %c\n", *fmtdex);
				FREE(ret);
				return NULL;
		}

		FREE(prtstr);
#undef PRINT
#undef PRINTF
	}

	*retdex = '\0';
	return ret;
}
/****************************************************************************/
/* }}}                                                                      */
/****************************************************************************/

static enum ArgCode short_arg(char argl)
{
	switch(argl) {
		case 'd':
			do_dozenal = true;
			return Aok;
		case 'D':
			do_dozenal = false;
			return Aok;
		case 'V':
			/* Do version */
			printf("%s (%s)\n", progname, VERSION);
			return Quit;
		case 'h':
			/* Do help */
			printf("usage: %s [options] [+format]\n", progname);
			return Quit;
		default:
			/* Do usage */
			fprintf(stderr, "Unknown arg %c!\n", argl);
			fprintf(stderr, "usage: %s [options] [+format]\n", progname);
			return Fail;
	}
}

static enum ArgCode short_args(char *argl)
{
	enum ArgCode rc;
	for (; *argl; argl++) {
		rc = short_arg(*argl);
		if (rc != Aok)
			return rc;
	}
	return Aok;
}

int main(int argc, char **argv)
{
	struct RDate date;
	char *time_str = NULL;
	char *fmt_str = NULL;
	int ii;
	char ret = 0;
	char *p;

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
				switch (short_args(argv[ii] + 1)) {
					case Fail:
						ret = 1;
					case Quit:
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
		fmt_str = "%A, %E %B, %Y";

	/************************************************/
	/* Prepare time                                 */
	/************************************************/
	date = get_date();
	time_str = date_str_fmt(fmt_str, &date);

	if (time_str)
		printf("%s\n", time_str);

CLEANUP:

	FREE(time_str);

	return ret;
}

