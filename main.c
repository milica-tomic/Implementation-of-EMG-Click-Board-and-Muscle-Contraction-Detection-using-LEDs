

#include "mcc_generated_files/mcc.h"
#include <string.h>
#define N 20
volatile bool flag = false;

int16_t srednja_vrednost (int16_t *niz, int16_t n)
 {
    int32_t suma = 0;
    for ( int8_t i=0; i <n; i++)
    suma += niz[i];

    return ( int16_t )( suma/n);
 }

int16_t standardna_devijacija (int16_t *niz, int16_t srednja_vrednost,int16_t n)
 {
    int32_t suma = 0;
    for ( int8_t i=0; i <n; i++)
    suma += (niz[i] - srednja_vrednost)*(niz[i] - srednja_vrednost);

    return ( int16_t )( suma/n);
 }

void swap(int *a, int *b) 
{ 
    int temp = *a; 
    *a = *b; 
    *b = temp; 
}  

void Sort(int16_t *niz, int16_t n) 
{ 
    int i, j; 
    for (i = 0; i < n-1; i++)       
        for (j = 0; j < n-i-1; j++) 
            if (niz[j] > niz[j+1]) 
                swap(&niz[j], &niz[j+1]); 
}   

/*
int16_t modus(int16_t *niz)
{
    int i, j, count;
    int maxCount = 0;
    int16_t maxElement;
    
   
    for(i = 0; i< 5; i++)  
    {
        count = 1;
        for(j = i+1; j < 5; j++)  
        {
            if(niz[j] == niz[i])
            {
                count++;     
                
                if(count > maxCount)
                {
                    maxCount = count;
                    maxElement = niz[j];
                }
            }
        }
    }
    return maxElement;
}
*/
typedef enum { cekanje , vece , manje, pauza} state_t ;
state_t  stanje;

void main(void)
{ 
    SYSTEM_Initialize();

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    uint16_t adc[20];
    uint8_t i=0;
    uint16_t adc_srednje;
    int16_t konacno[20];
    int16_t s_devijacija;
    int16_t srednja;
    int16_t mediana;
   
   
   
    stanje = cekanje;
    
    memset(adc , 0 , 20);
    memset(konacno , 0 , 20);
    i=0;

    ADC_StartConversion(channel_AN1);
    ECCP2_SetCompareCount(0x4E20); /* na 100Hz*/
    
    LATD = 0x00;
    LATB = 0x00;
    INTCONbits.TMR0IE = 0;
    
    while (1)
    {
        if(flag_adc){
            
            flag_adc = false;
            adc[i] = ADC_GetConversionResult();
            adc_srednje = srednja_vrednost(adc, N);
            
            konacno[i] = adc[i] - adc_srednje; 
            srednja = srednja_vrednost(konacno, N);
           
            //memcpy(usrednjavanje,konacno,20);
            
            /*======MEDIANA======*/
            Sort(konacno, N);
            mediana = konacno[N/2];
            
            /*
            if (mediana > 130 )
            {
                LATD = 0xFF;
                
            }
            else {LATD = 0x00;}
            
            if (mediana <100 && mediana > 50 )
            {
                LATB = 0xFF;
                       
            }
            else {LATB = 0x00;}
            */
            
            /*======STANDARDNA DEVIJACIJA======*/
            s_devijacija = standardna_devijacija(konacno, srednja, N);
            
            EUSART1_Write(0x03);
            EUSART1_Write((uint8_t)adc[i]);
            EUSART1_Write((uint8_t)(adc[i]>>8));
            EUSART1_Write((uint8_t)konacno[i]);
            EUSART1_Write((uint8_t)(konacno[i]>>8));
             EUSART1_Write((uint8_t)srednja);
            EUSART1_Write((uint8_t)(srednja>>8));
            EUSART1_Write((uint8_t)s_devijacija);
            EUSART1_Write((uint8_t)(s_devijacija>>8));
            EUSART1_Write((uint8_t)mediana);
            EUSART1_Write((uint8_t)(mediana>>8));
            EUSART1_Write(0xFC);
            
            
            switch(stanje)
            {
                case cekanje:
                    
                    LATB = 0x00;
                    LATD = 0x00;
                    if (mediana > 130)
                    {
                        stanje = vece;
                    
                    }
                    
                    if (mediana <90 && mediana > 60)
                    {
                        stanje = manje;
                      
                    }
                    break;
                    
                case vece:
                    LATD = 0xFF;
                    LATB = 0x00;
                    //__delay_ms (1000);
                    stanje = pauza;
                    TMR0_Reload();
                    INTCONbits.TMR0IE = 1;
                    break;
       
                case manje:
                    LATB = 0xFF;
                    LATD = 0x00;
                    //__delay_ms (1000);
                    stanje = pauza;
                    TMR0_Reload();
                    INTCONbits.TMR0IE = 1;
                    break;
                    
                case pauza:
     
                    if(flag)
                    {
                        flag = false;
                        INTCONbits.TMR0IE = 0;
                        
                        stanje = cekanje;
                    }
                    break;
                default:
                    stanje = cekanje;  
                    
            }
            
             
          
        } 
        i++;
        /*ako dodje nova vrednost stavi je na mestu najstarije vrednosti*/
        if (i == 20)
        {
            i = 0;
        }

    }
}