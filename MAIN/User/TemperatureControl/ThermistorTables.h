#ifndef THERMISTORTABLES_H
#define THERMISTORTABLES_H

#define THERMISTORHEATER_0      SEMITEC_104NT_4_R025H43G
#define THERMISTORHEATER_1      SEMITEC_104NT_4_R025H43G
#define THERMISTORHEATER_2      SEMITEC_104NT_4_R025H43G
#define THERMISTORBED           SEMITEC_104NT_4_R025H43G

#define OVERSAMPLENR 8

#define SEMITEC_104NT_4_R025H43G    1

#if (THERMISTORHEATER_0 == SEMITEC_104NT_4_R025H43G) || (THERMISTORHEATER_1 == SEMITEC_104NT_4_R025H43G) || (THERMISTORHEATER_2 == SEMITEC_104NT_4_R025H43G) || (THERMISTORBED == SEMITEC_104NT_4_R025H43G) // SEMITEC_104NT-4-R025H43G with 4.7k Resistor

#if (THERMISTORHEATER_0 == SEMITEC_104NT_4_R025H43G)
# define HEATER_0_RAW_HI_TEMP 0
# define HEATER_0_RAW_LO_TEMP 4095
#endif
#if (THERMISTORHEATER_1 == SEMITEC_104NT_4_R025H43G)
# define HEATER_1_RAW_HI_TEMP 0
# define HEATER_1_RAW_LO_TEMP 4095
#endif
#if (THERMISTORHEATER_2 == SEMITEC_104NT_4_R025H43G)
# define HEATER_2_RAW_HI_TEMP 0
# define HEATER_2_RAW_LO_TEMP 4095
#endif
#if (THERMISTORBED == SEMITEC_104NT_4_R025H43G)
# define HEATER_BED_RAW_HI_TEMP 0
# define HEATER_BED_RAW_LO_TEMP 4095
#endif

const uint16_t temptable_1[][2] = {
{       58*OVERSAMPLENR ,       300     }, /*{采样参考值, 对应温度}*/
{       62*OVERSAMPLENR ,       295     },
{       67*OVERSAMPLENR ,       290     },
{       72*OVERSAMPLENR ,       285     },
{       78*OVERSAMPLENR ,       280     },
{       84*OVERSAMPLENR ,       275     },
{       91*OVERSAMPLENR ,       270     },
{       98*OVERSAMPLENR ,       265     },
{       106*OVERSAMPLENR    ,       260     },
{       115*OVERSAMPLENR    ,       255     },
{       125*OVERSAMPLENR    ,       250     },
{       136*OVERSAMPLENR    ,       245     },
{       148*OVERSAMPLENR    ,       240     },
{       161*OVERSAMPLENR    ,       235     },
{       176*OVERSAMPLENR    ,       230     },
{       192*OVERSAMPLENR    ,       225     },
{       211*OVERSAMPLENR    ,       220     },
{       231*OVERSAMPLENR    ,       215     },
{       253*OVERSAMPLENR    ,       210     },
{       279*OVERSAMPLENR    ,       205     },
{       307*OVERSAMPLENR    ,       200     },
{       338*OVERSAMPLENR    ,       195     },
{       373*OVERSAMPLENR    ,       190     },
{       411*OVERSAMPLENR    ,       185     },
{       455*OVERSAMPLENR    ,       180     },
{       503*OVERSAMPLENR    ,       175     },
{       557*OVERSAMPLENR    ,       170     },
{       617*OVERSAMPLENR    ,       165     },
{       683*OVERSAMPLENR    ,       160     },
{       757*OVERSAMPLENR    ,       155     },
{       838*OVERSAMPLENR    ,       150     },
{       928*OVERSAMPLENR    ,       145     },
{       1027*OVERSAMPLENR       ,       140     },
{       1135*OVERSAMPLENR       ,       135     },
{       1253*OVERSAMPLENR       ,       130     },
{       1380*OVERSAMPLENR       ,       125     },
{       1516*OVERSAMPLENR       ,       120     },
{       1661*OVERSAMPLENR       ,       115     },
{       1813*OVERSAMPLENR       ,       110     },
{       1972*OVERSAMPLENR       ,       105     },
{       2135*OVERSAMPLENR       ,       100     },
{       2302*OVERSAMPLENR       ,       95      },
{       2470*OVERSAMPLENR       ,       90      },
{       2635*OVERSAMPLENR       ,       85      },
{       2797*OVERSAMPLENR       ,       80      },
{       2952*OVERSAMPLENR       ,       75      },
{       3099*OVERSAMPLENR       ,       70      },
{       3236*OVERSAMPLENR       ,       65      },
{       3362*OVERSAMPLENR       ,       60      },
{       3475*OVERSAMPLENR       ,       55      },
{       3577*OVERSAMPLENR       ,       50      },
{       3665*OVERSAMPLENR       ,       45      },
{       3743*OVERSAMPLENR       ,       40      },
{       3808*OVERSAMPLENR       ,       35      },
{       3864*OVERSAMPLENR       ,       30      },
{       3911*OVERSAMPLENR       ,       25      },
{       3950*OVERSAMPLENR       ,       20      },
{       3981*OVERSAMPLENR       ,       15      },
{       4007*OVERSAMPLENR       ,       10      },
{       4027*OVERSAMPLENR       ,       5       },
{       4043*OVERSAMPLENR       ,       0       } //safety
};
#endif

#define _TT_NAME(_N) temptable_ ## _N
#define TT_NAME(_N) _TT_NAME(_N)

#ifdef THERMISTORHEATER_0
# define HEATER_0_TEMPTABLE TT_NAME(THERMISTORHEATER_0)
# define HEATER_0_TEMPTABLE_LEN (sizeof(HEATER_0_TEMPTABLE)/sizeof(*HEATER_0_TEMPTABLE))
#else
# ifdef HEATER_0_USES_THERMISTOR
#  error No heater 0 thermistor table specified
# else  // HEATER_0_USES_THERMISTOR
#  define HEATER_0_TEMPTABLE NULL
#  define HEATER_0_TEMPTABLE_LEN 0
# endif // HEATER_0_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_0_RAW_HI_TEMP
# ifdef HEATER_0_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_0_RAW_HI_TEMP 0
#  define HEATER_0_RAW_LO_TEMP 4095
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_0_RAW_HI_TEMP 4095
#  define HEATER_0_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORHEATER_1
# define HEATER_1_TEMPTABLE TT_NAME(THERMISTORHEATER_1)
# define HEATER_1_TEMPTABLE_LEN (sizeof(HEATER_1_TEMPTABLE)/sizeof(*HEATER_1_TEMPTABLE))
#else
# ifdef HEATER_1_USES_THERMISTOR
#  error No heater 1 thermistor table specified
# else  // HEATER_1_USES_THERMISTOR
#  define HEATER_1_TEMPTABLE NULL
#  define HEATER_1_TEMPTABLE_LEN 0
# endif // HEATER_1_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_1_RAW_HI_TEMP
# ifdef HEATER_1_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_1_RAW_HI_TEMP 0
#  define HEATER_1_RAW_LO_TEMP 4095
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_1_RAW_HI_TEMP 4095
#  define HEATER_1_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORHEATER_2
# define HEATER_2_TEMPTABLE TT_NAME(THERMISTORHEATER_2)
# define HEATER_2_TEMPTABLE_LEN (sizeof(HEATER_2_TEMPTABLE)/sizeof(*HEATER_2_TEMPTABLE))
#else
# ifdef HEATER_2_USES_THERMISTOR
#  error No heater 2 thermistor table specified
# else  // HEATER_2_USES_THERMISTOR
#  define HEATER_2_TEMPTABLE NULL
#  define HEATER_2_TEMPTABLE_LEN 0
# endif // HEATER_2_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_2_RAW_HI_TEMP
# ifdef HEATER_2_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_2_RAW_HI_TEMP 0
#  define HEATER_2_RAW_LO_TEMP 4095
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_2_RAW_HI_TEMP 4095
#  define HEATER_2_RAW_LO_TEMP 0
# endif
#endif

#ifdef THERMISTORBED
# define BEDTEMPTABLE TT_NAME(THERMISTORBED)
# define BEDTEMPTABLE_LEN (sizeof(BEDTEMPTABLE)/sizeof(*BEDTEMPTABLE))
#else
# ifdef BED_USES_THERMISTOR
#  error No bed thermistor table specified
# endif // BED_USES_THERMISTOR
#endif

//Set the high and low raw values for the heater, this indicates which raw value is a high or low temperature
#ifndef HEATER_BED_RAW_HI_TEMP
# ifdef BED_USES_THERMISTOR   //In case of a thermistor the highest temperature results in the lowest ADC value
#  define HEATER_BED_RAW_HI_TEMP 0
#  define HEATER_BED_RAW_LO_TEMP 4095
# else                          //In case of an thermocouple the highest temperature results in the highest ADC value
#  define HEATER_BED_RAW_HI_TEMP 4095
#  define HEATER_BED_RAW_LO_TEMP 0
# endif
#endif


#endif /* THERMISTORTABLES_H */
