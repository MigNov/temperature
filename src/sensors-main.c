//#define DEBUG

#include <signal.h>
#include "sensors.h"

#ifdef DEBUG
#define DPRINTF(fmt, ...) \
do { fprintf(stderr, "sensors: " fmt , ## __VA_ARGS__); } while (0)
#else
#define DPRINTF(fmt, ...) \
do {} while(0)
#endif

#define FLAG_OUTPUT_XML 1
#define FLAG_OUTPUT_AVG 2
#define FLAG_OUTPUT_MAX 4
#define FLAG_RETURN_EOE 8
#define FLAG_USE_XML	16

#define FLAG_OUTPUT_DBG 128
#define MAX_TEMP_SHIFT  8

char sCmd[1024] = { 0 };
char sXmlFile[1024] = "/etc/temperature-settings.xml";

void dumpFlags(unsigned long flags) {
    if (!(flags & FLAG_OUTPUT_DBG))
        return ;
    printf("\nDumping flags:\n");
    printf("\tOutput XML: %s\n",
        flags & FLAG_OUTPUT_XML ? "True" : "False");
    printf("\tOutput average value: %s\n",
        flags & FLAG_OUTPUT_AVG ? "True" : "False");
    printf("\tMaximum temperature set: %s\n",
        flags & FLAG_OUTPUT_MAX ? "True" : "False");
    printf("\tPCI support: ");
#ifdef HAVE_PCI
    printf("Compiled\n");
#else
    printf("Not compiled\n");
#endif

    printf("\tUse XML processing: %s\n",
        flags & FLAG_USE_XML ? "True" : "False");
    if (flags & FLAG_USE_XML)
        printf("\tXML definition file: %s\n", sXmlFile);

    if (flags & FLAG_OUTPUT_MAX) {
        printf("\t  Maximum temperature: %d °C\n", (int)(flags >> MAX_TEMP_SHIFT));
        printf("\t  Command when exceeded: %s\n", sCmd);
    }

    printf("\tReturn OS error code on exit: %s\n",
        flags & FLAG_RETURN_EOE ? "True" : "False");

    printf("\n");
}

int getFlagSet(unsigned int flag, unsigned short bit) {
    return (flag & bit) ? 1 : 0;
}

unsigned int setMaximumTemperature(char *str) {
    int val;
    char *endptr;

    val = strtol(str, &endptr, 10);
    return ((val > 0) && (errno == 0)) ? FLAG_OUTPUT_MAX | ( val << MAX_TEMP_SHIFT ) : 0;
}

unsigned int getMaximumTemperature(unsigned long flags) {
    if (!getFlagSet(flags, FLAG_OUTPUT_MAX))
        return 0;

    return flags >> MAX_TEMP_SHIFT;
}

int parseFlags(int argc, char * const argv[]) {
    int option_index = 0, c;
    unsigned int retVal = 0;
    static struct option long_options[] = {
        {"xml", 0, 0, 'x'},
        {"average", 0, 0, 'a'},
        {"debug", 0, 0, 'd'},
        {"command", 1, 0, 'c'},
        {"max-temp", 1, 0, 'm'},
        {"error-on-exit", 0, 0, 'e'},
        {"use-xml", 0, 0, 'u'},
        {"xml-definition-file", 1, 0, 'f'},
        {0, 0, 0, 0}
    };

    while (1) {
        c = getopt_long(argc, argv, "xadem:c:u:",
                   long_options, &option_index);
        if (c == -1)
            break;

        switch (c) {
            case 'x': retVal |= FLAG_OUTPUT_XML;
                      break;
            case 'a': retVal |= FLAG_OUTPUT_AVG;
                      break;
            case 'd': retVal |= FLAG_OUTPUT_DBG;
                      break;
            case 'e': retVal |= FLAG_RETURN_EOE;
                      break;
            case 'u': retVal |= FLAG_USE_XML;
                      break;
            case 'f': strncpy(sXmlFile, optarg, sizeof(sXmlFile));
                      break;
            case 'm': retVal |= setMaximumTemperature(optarg);
                      break;
            case 'c': strncpy(sCmd, optarg, sizeof(sCmd));
                      break;

            default : break;
        }
    }

    return retVal;
}

void printOutput(unsigned long flags, int val) {
    int xml = getFlagSet(flags, FLAG_OUTPUT_XML);

    if (val > 0) {
        if (xml) {
            printf("<report type=\"result\">\n    <temperature average=\"%d\">%d</temperature>\n</report>\n",
                    getFlagSet(flags, FLAG_OUTPUT_AVG), val);
        }
        else {
            printf("%s temperature: %d °C\n", getFlagSet(flags, FLAG_OUTPUT_AVG) ? "Average" : "Highest", val);
        }
    }
    else {
        val = -val;
        if (xml) {
            printf("<report type=\"error\">\n    <error code=\"%d\">%s</error>\n</report>\n", val,
                   strerror(val));
        }
        else {
            printf("Error getting temperature: %s (code %d)\n", strerror(val), val);
        }
    }
}

/* Return 0 on success, 1 on error */
int checkFlagsSanity(unsigned long flags) {
    int xml, dbg, max;

    xml = getFlagSet(flags, FLAG_OUTPUT_XML);
    max = getFlagSet(flags, FLAG_OUTPUT_MAX);
    dbg = getFlagSet(flags, FLAG_OUTPUT_DBG);

    if (getFlagSet(flags, FLAG_OUTPUT_MAX)
      && ((strlen(sCmd) == 0) || access(sCmd, X_OK) != 0) ) {
        if (strlen(sCmd) == 0)
            fprintf(stderr, "Error: Command definition is missing\n");
        else
            fprintf(stderr, "Error: Command %s doesn't exist or command "
                            "is not executable\n", sCmd);

        return 1;
    }

    DPRINTF("checkFlagsSanity: xml = %d, max = %d, dbg = %d\n", xml, max, dbg);
    return (xml && (max || dbg));
}

int getTemperature(int average, int bIntel) {
    int val;

    if (bIntel < 0)
        CPUGetModel(&bIntel);

    if (bIntel)
        val = iIntelGetTemp( average );
    else
#ifdef HAVE_PCI
        val = iAMDGetTempK10( average );
#else
	val = -ENOTSUP;
#endif

    return val;
}

/* 
 * Returns zero if it can terminate, current temperature value if greater than maxVal
 * or error value if negative temperature value
 */
int loopSensors(int interval, unsigned long flags) {
    int bIntel, val, maxVal, average;

    CPUGetModel(&bIntel);

    maxVal = getMaximumTemperature(flags);
    average = getFlagSet(flags, FLAG_OUTPUT_AVG);

    while (1) {
        val = getTemperature(average, bIntel);
        if ((val < 0) || (val > maxVal))
            return val;
        sleep(interval);
    }

    return 0;
}

int main(int argc, char *argv[]) {
    char cmd[1024] = { 0 };
    int bIntel, val;
    unsigned long flags;
    time_t old_time, sec_time;

    flags = parseFlags(argc, argv);
    dumpFlags( flags );

    if (checkFlagsSanity( flags ) != 0) {
        fprintf(stderr, "Incorrect flag combination set. Quiting ...\n");
        return EINVAL;
    }

    if (getFlagSet(flags, FLAG_USE_XML)) {
        processXml(sXmlFile);

        return 0;
    }

    if (getMaximumTemperature(flags) > 0) {
        old_time = time(NULL);
        printf("Temperature watch started...\n");
        while ((val = loopSensors(1, flags)) > 0) {
            sec_time = time(NULL) - old_time;
            old_time = time(NULL);
            if ((int)sec_time > 0) {
                snprintf(cmd, sizeof(cmd), "%s %d %d", sCmd, val, (int)sec_time);
                val = system(cmd);
                if (WIFEXITED(val)) {
                    val >>= 8;
                    DPRINTF("Command return value %d\n", val);
                    if (val > 0)
                        return val;
                }
                if (WIFSIGNALED(val) && (WTERMSIG(val) != SIGINT
                     && WTERMSIG(val) != SIGQUIT)) {
                        fprintf(stderr, "Command %s signaled. Quiting ...\n", sCmd);
                        return EINVAL;
                }
            }
            sleep(1);
        }

        if (val < 0)
            fprintf(stderr, "An error occured: %s (%d)\n", strerror(-val), -val);

        return (val > 0) ? 0 : (getFlagSet(flags, FLAG_RETURN_EOE) ? -val : 1);
    }
    else {
        CPUGetModel(&bIntel);

        if (bIntel)
		val = iIntelGetTemp( getFlagSet(flags, FLAG_OUTPUT_AVG) );
        else
#ifdef HAVE_PCI
		val = iAMDGetTempK10( getFlagSet(flags, FLAG_OUTPUT_AVG) );
#else
		val = -ENOTSUP;
#endif

        printOutput(flags, val);

	if (val == -ENOTSUP)
		return -ENOTSUP;

        return (val > 0) ? 0 : (getFlagSet(flags, FLAG_RETURN_EOE) ? -val : 1);
    }
}

