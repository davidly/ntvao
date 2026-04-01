/*
 * WUMPUS for old CP/M C compilers such as Manx Aztec C.
 *
 * Conservative choices:
 * - K&R syntax
 * - no stdlib.h
 * - no string.h
 * - short identifiers for 8-char significance
 * - no void
 */

#include <stdio.h>
#include <ctype.h>

#define A1_SUPPORT_SHOW    0
#define A1_SUPPORT_LONG    0
#define A1_SUPPORT_KBD     1
#define A1_SUPPORT_PR_SXX  1
#include "a1.c" /* apple 1 support routines */

char g_buf[64];

#define RMCNT 20
#define TUNC  3
#define LOCC  6
#define ARRMX 5
#define ARRWS 5

#define true 1
#define false 0

#define NO  0
#define YES 1

#define EPLY 0
#define EWUM 1
#define EP1  2
#define EP2  3
#define EB1  4
#define EB2  5

#define TMOV 0
#define TSHO 1

#define OCON 0
#define OWIN 1
#define OLOS 2

typedef struct Gm {
    int loc[LOCC];
    int ini[LOCC];
    int arr;
    int cpu;
} Gm;

static int cave[RMCNT + 1][TUNC] = {
    {0, 0, 0},
    {2, 5, 8},
    {1, 3, 10},
    {2, 4, 12},
    {3, 5, 14},
    {1, 4, 6},
    {5, 7, 15},
    {6, 8, 17},
    {1, 7, 9},
    {8, 10, 18},
    {2, 9, 11},
    {10, 12, 19},
    {3, 11, 13},
    {12, 14, 20},
    {4, 13, 15},
    {6, 14, 16},
    {15, 17, 20},
    {7, 16, 18},
    {9, 17, 19},
    {11, 18, 20},
    {13, 16, 19}
};

static int seed = 17;

static int rnd16();
static int rndrm();
static int rndix();
static int streq();
static int cpint();
static int rdlin();
static int rdchr();
static int rdnum();
static int adjo();
static int dupck();
static int hpit();
static int hbat();
static int hwum();
static int safeo();
static int inst();
static int ginit();
static int gsame();
static int warns();
static int stats();
static int wwak();
static int presv();
static int pact();
static int pmove();
static int movto();
static int plen();
static int ppath();
static int ashot();
static int pshot();
static int pins();
static int psame();
static int rmove();
static int smove();
static int fwum();
static int cact();
static int cturn();

static int rnd16()
{
    seed = seed * 25173 + 13849;
    return (seed >> 1) & 0x7fff;
}

static int rndrm()
{
    return (rnd16() % RMCNT) + 1;
}

static int rndix(n)
int n;
{
    if (n <= 0)
        return 0;
    return rnd16() % n;
}

static int streq(a, b)
char *a;
char *b;
{
    int ca, cb;

    while (*a != '\0' && *b != '\0') {
        ca = toupper(*a);
        cb = toupper(*b);
        if (ca != cb)
            return NO;
        ++a;
        ++b;
    }

    return (*a == '\0' && *b == '\0');
}

static int cpint(d, s, n)
int *d;
int *s;
int n;
{
    int i;

    for (i = 0; i < n; ++i)
        d[i] = s[i];

    return 0;
}

static int rdlin(buf, siz)
char *buf;
int siz;
{
    int i;
    int ch;

    if (get_input(buf, siz) == 0) {
        buf[0] = '\0';
        return 0;
    }

    for (i = 0; buf[i] != '\0'; ++i) {
        ch = buf[i];
        if (ch == '\n' || ch == '\r') {
            buf[i] = '\0';
            break;
        }
    }

    return 0;
}

static int rdchr()
{
    int i, ch;

    rdlin(g_buf, sizeof(g_buf));

    for (i = 0; g_buf[i] != '\0'; ++i) {
        ch = g_buf[i];
        if (!isspace(ch))
            return toupper(ch);
    }

    return '\0';
}

static int rdnum()
{
    int i, ch, neg, val, got;

    for (;;) {
        rdlin(g_buf, sizeof(g_buf));

        i = 0;
        while (g_buf[i] != '\0' && isspace(g_buf[i]))
            ++i;

        neg = 0;
        if (g_buf[i] == '-') {
            neg = 1;
            ++i;
        }

        val = 0;
        got = 0;
        while (g_buf[i] != '\0') {
            ch = g_buf[i];
            if (!isdigit(ch))
                break;
            val = val * 10 + (ch - '0');
            got = 1;
            ++i;
        }

        if (got) {
            if (neg)
                val = -val;
            return val;
        }

        pr_s("? ");
    }
}

static int adjo(f, t)
int f;
int t;
{
    int i;

    for (i = 0; i < TUNC; ++i) {
        if (cave[f][i] == t)
            return YES;
    }

    return NO;
}

static int dupck(v, n)
int *v;
int n;
{
    int i, j;

    for (i = 0; i < n; ++i)
        for (j = i + 1; j < n; ++j)
            if (v[i] == v[j])
                return YES;

    return NO;
}

static int hpit(g, r)
Gm *g;
int r;
{
    return (r == g->loc[EP1] || r == g->loc[EP2]);
}

static int hbat(g, r)
Gm *g;
int r;
{
    return (r == g->loc[EB1] || r == g->loc[EB2]);
}

static int hwum(g, r)
Gm *g;
int r;
{
    return (r == g->loc[EWUM]);
}

static int safeo(g, r)
Gm *g;
int r;
{
    return !hpit(g, r) && !hbat(g, r) && !hwum(g, r);
}

static int inst()
{
    pr_s("WELCOME TO 'HUNT THE WUMPUS'\n");
    pr_s("THE WUMPUS LIVES IN A CAVE OF 20 ROOMS. EACH ROOM\n");
    pr_s("HAS 3 TUNNELS LEADING TO OTHER ROOMS.\n");
    pr_s("\n");
    pr_s("HAZARDS:\n");
    pr_s("  BOTTOMLESS PITS - TWO ROOMS HAVE PITS.\n");
    pr_s("      GO THERE AND YOU LOSE.\n");
    pr_s("  SUPER BATS - TWO ROOMS HAVE BATS.\n");
    pr_s("      THEY MAY CARRY YOU TO A RANDOM ROOM.\n");
    pr_s("\n");
    pr_s("WUMPUS:\n");
    pr_s("  ENTER HIS ROOM OR SHOOT TO WAKE HIM.\n");
    pr_s("  WHEN WAKENED, HE MAY MOVE OR STAY PUT.\n");
    pr_s("  IF HE ENDS UP IN YOUR ROOM, YOU LOSE.\n");
    pr_s("\n");
    pr_s("YOU:\n");
    pr_s("  EACH TURN YOU MAY MOVE OR SHOOT.\n");
    pr_s("  YOU HAVE 5 ARROWS.\n");
    pr_s("  EACH ARROW MAY TRAVEL THROUGH 1 TO 5 ROOMS.\n");
    pr_s("  IF AN ARROW HITS THE WUMPUS, YOU WIN.\n");
    pr_s("  IF IT HITS YOU, YOU LOSE.\n");
    pr_s("\n");
    pr_s("WARNINGS:\n");
    pr_s("  WUMPUS - I SMELL A WUMPUS\n");
    pr_s("  BATS   - BATS NEARBY\n");
    pr_s("  PIT    - I FEEL A DRAFT\n");
    pr_s("\n");
    return 0;
}

static int ginit(g)
Gm *g;
{
    int i;

    do {
        for (i = 0; i < LOCC; ++i)
            g->loc[i] = rndrm();
    } while (dupck(g->loc, LOCC));

    cpint(g->ini, g->loc, LOCC);
    g->arr = ARRWS;
    return 0;
}

static int gsame(g)
Gm *g;
{
    cpint(g->loc, g->ini, LOCC);
    g->arr = ARRWS;
    return 0;
}

static int warns(g)
Gm *g;
{
    int rm, i, a;

    rm = g->loc[EPLY];

    for (i = 0; i < TUNC; ++i) {
        a = cave[rm][i];
        if (a == g->loc[EWUM])
            pr_s("I SMELL A WUMPUS!\n");
        if (a == g->loc[EP1] || a == g->loc[EP2])
            pr_s("I FEEL A DRAFT!\n");
        if (a == g->loc[EB1] || a == g->loc[EB2])
            pr_s("BATS NEARBY!\n");
    }

    return 0;
}

static int stats(g)
Gm *g;
{
    int rm;

    rm = g->loc[EPLY];
    pr_s("\n");
    warns(g);
    pr_sis("YOU ARE IN ROOM ", rm, "\n");
    pr_sis("TUNNELS LEAD TO ", cave[rm][0], 0);
    pr_sis(", ", cave[rm][1], 0 );
    pr_sis(", AND ", cave[rm][2], "\n" );
    pr_sis("ARROWS LEFT: ", g->arr, "\n");
    pr_s("\n");
    return 0;
}

static int wwak(g)
Gm *g;
{
    int mv, wr;

    mv = rndix(4);
    if (mv < 3) {
        wr = g->loc[EWUM];
        g->loc[EWUM] = cave[wr][mv];
    }

    if (g->loc[EWUM] == g->loc[EPLY]) {
        pr_s("TSK TSK TSK - WUMPUS GOT YOU!\n");
        return OLOS;
    }

    return OCON;
}

static int presv(g)
Gm *g;
{
    int rm;

    for (;;) {
        rm = g->loc[EPLY];

        if (rm == g->loc[EWUM]) {
            pr_s("... OOPS! BUMPED A WUMPUS!\n");
            return wwak(g);
        }

        if (rm == g->loc[EP1] || rm == g->loc[EP2]) {
            pr_s("YYYIIIIEEEE . . . FELL IN PIT\n");
            return OLOS;
        }

        if (rm == g->loc[EB1] || rm == g->loc[EB2]) {
            pr_s("ZAP--SUPER BAT SNATCH! ELSEWHEREVILLE FOR YOU!\n");
            g->loc[EPLY] = rndrm();
            continue;
        }

        return OCON;
    }
}

static int pact()
{
    int ch;

    for (;;) {
        pr_s("SHOOT OR MOVE (S-M) ");
        ch = rdchr();
        if (ch == 'S')
            return TSHO;
        if (ch == 'M')
            return TMOV;
    }
}

static int movto(g, dst)
Gm *g;
int dst;
{
    g->loc[EPLY] = dst;
    return presv(g);
}

static int pmove(g)
Gm *g;
{
    int dst;

    for (;;) {
        pr_s("WHERE TO ");
        dst = rdnum();

        if (dst < 1 || dst > RMCNT) {
            pr_s("NOT POSSIBLE -\n");
            continue;
        }

        if (!adjo(g->loc[EPLY], dst)) {
            pr_s("NOT POSSIBLE -\n");
            continue;
        }

        break;
    }

    return movto(g, dst);
}

static int plen()
{
    int n;

    for (;;) {
        pr_s("NO. OF ROOMS (1-5) ");
        n = rdnum();
        if (n >= 1 && n <= ARRMX)
            return n;
    }
}

static int ppath(pth, len)
int *pth;
int len;
{
    int i;

    for (i = 0; i < len; ++i) {
        for (;;) {
            pr_sis("ROOM # ", i + 1, " ");
            pth[i] = rdnum();

            if (pth[i] < 1 || pth[i] > RMCNT)
                continue;

            if (i >= 2 && pth[i] == pth[i - 2]) {
                pr_s("ARROWS AREN'T THAT CROOKED - TRY ANOTHER ROOM\n");
                continue;
            }

            break;
        }
    }

    return 0;
}

static int ashot(g, pth, len)
Gm *g;
int *pth;
int len;
{
    int ar, i;

    ar = g->loc[EPLY];

    for (i = 0; i < len; ++i) {
        if (adjo(ar, pth[i]))
            ar = pth[i];
        else
            ar = cave[ar][rndix(TUNC)];

        if (ar == g->loc[EWUM]) {
            pr_s("AHA! YOU GOT THE WUMPUS!\n");
            return OWIN;
        }

        if (ar == g->loc[EPLY]) {
            pr_s("OUCH! ARROW GOT YOU!\n");
            g->arr = g->arr - 1;
            return OLOS;
        }
    }

    g->arr = g->arr - 1;
    pr_s("MISSED\n");

    if (g->arr <= 0) {
        pr_s("YOU RAN OUT OF ARROWS!\n");
        return OLOS;
    }

    return wwak(g);
}

static int pshot(g)
Gm *g;
{
    int pth[ARRMX];
    int len;

    len = plen();
    ppath(pth, len);
    return ashot(g, pth, len);
}

static int pins()
{
    pr_s("INSTRUCTIONS (Y-N) ");
    return (rdchr() != 'N');
}

static int pcomp()
{
    pr_s("COMPUTER PLAYS (Y-N) ");
    return (rdchr() != 'N');
}

static int psame()
{
    pr_s("SAME SET-UP (Y-N) ");
    return (rdchr() == 'Y');
}

static int rmove(g)
Gm *g;
{
    int rm;

    rm = g->loc[EPLY];
    return cave[rm][rndix(TUNC)];
}

static int smove(g)
Gm *g;
{
    int rm, lst[TUNC], cnt, i, r;

    rm = g->loc[EPLY];
    cnt = 0;

    for (i = 0; i < TUNC; ++i) {
        r = cave[rm][i];
        if (safeo(g, r))
            lst[cnt++] = r;
    }

    if (cnt == 0)
        return 0;

    return lst[rndix(cnt)];
}

static int fwum(g, out)
Gm *g;
int *out;
{
    int rm, i, r;

    rm = g->loc[EPLY];

    for (i = 0; i < TUNC; ++i) {
        r = cave[rm][i];
        if (r == g->loc[EWUM]) {
            *out = r;
            return YES;
        }
    }

    return NO;
}

static int cact(g)
Gm *g;
{
    int wr;

    if (g->arr > 0 && fwum(g, &wr))
        return TSHO;

    return TMOV;
}

static int cturn(g)
Gm *g;
{
    int act, trg, pth[1], dst;

    act = cact(g);

    if (act == TSHO) {
        trg = 0;
        fwum(g, &trg);
        pth[0] = trg;

        pr_s("COMPUTER CHOOSES: SHOOT\n");
        pr_sis("COMPUTER SHOOTS THROUGH ROOM ", trg, "\n");

        return ashot(g, pth, 1);
    }

    dst = smove(g);
    if (dst == 0)
        dst = rmove(g);

    pr_s("COMPUTER CHOOSES: MOVE\n");
    pr_sis("COMPUTER MOVES TO ROOM ", dst, "\n");

    return movto(g, dst);
}

int main(ac, av)
int ac;
char **av;
{
    Gm g;
    int out, act;

    pr_s("            WUMPUS\n");
    pr_s(" CREATIVE COMPUTING MORRISTOWN, NJ\n");
    pr_s("\n");

    g.cpu = pcomp();

    ginit(&g);

    if (!g.cpu) {
        if (pins())
            inst();
    }

    out = OCON;

    pr_s("HUNT THE WUMPUS\n");

    while (out == OCON) {
        stats(&g);

        if (g.cpu)
            out = cturn(&g);
        else {
            act = pact();
            if (act == TMOV)
                out = pmove(&g);
            else
                out = pshot(&g);
        }
    }

    if (out == OWIN)
        pr_s("HEE HEE HEE - THE WUMPUS'LL GETCHA NEXT TIME!!\n");
    else
        pr_s("HA HA HA - YOU LOSE!\n");

    bye();
    return 0;
}
