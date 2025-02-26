#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2812.pio.h" // Biblioteca do WS2812

const uint led_pin_red = 13;    // LED vermelho na GPIO13
const uint led_pin_blue = 12;   // LED azul na GPIO12
const uint led_pin_green = 11;  // LED verde na GPIO11
const uint microfone = 28;      // GPIO28 para ADC2 (Microfone)
const uint limiar_1 = 2080;     // Limiar para o LED vermelho (som próximo)
const uint limiar_2 = 2200;     // Limiar para o LED verde (som distante)
const uint temp_sensor_pin = 4; // Canal ADC para o sensor de temperatura

#define NUM_PIXELS 25
#define WS2812_PIN 7 // GPIO para controlar WS2812
#define BTN_A_PIN 5
#define BTN_B_PIN 6
/*
24,23,22,21,20,
15,16,17,18,19,
14,13,12,11,10,
05,06,07,08,09,
04,03,02,01,00
*/

// Buffer para armazenar quais LEDs estão ligados na matriz 5x5
bool led_buffers[10][NUM_PIXELS] = {
    { 0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0 }, // 0
      
      { 0, 1, 1, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 0, 0,
        0, 0, 1, 0, 0 }, // 1
      
      { 0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0 }, // 2
      
      { 0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0}, // 3

      { 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,    
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0 }, // 4
      
       {0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0},// 5
      
      { 0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0}, // 6
      
      { 0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0 }, // 7

      
      { 0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0}, // 8
      
      { 0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0 }  // 9
};

int main()
{
    // Inicializa o sistema e o Serial Monitor
    stdio_init_all();

    // Inicializa a biblioteca WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, false);

    // Inicializa os LEDs RGB
    gpio_init(led_pin_red);
    gpio_set_dir(led_pin_red, GPIO_OUT);
    gpio_init(led_pin_green);
    gpio_set_dir(led_pin_green, GPIO_OUT);
    gpio_init(led_pin_blue);
    gpio_set_dir(led_pin_blue, GPIO_OUT);

    // Inicializa o ADC
    
    adc_init();
    adc_gpio_init(microfone);
    adc_select_input(2); // Canal 2 para o microfone
    adc_gpio_init(temp_sensor_pin);
    adc_select_input(4); // Canal 4 para o sensor de temperatura

    // Inicializa os botões
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);
    gpio_init(BTN_B_PIN);
    gpio_set_dir(BTN_B_PIN, GPIO_IN);
    gpio_pull_up(BTN_B_PIN);

    // Variáveis para controle da matriz de LEDs
    bool matriz_ativa = false;
    int numero_mostrado = 0;

    // Frequência de amostragem do microfone
    const uint amostras_por_segundo = 8000;
    uint64_t intervalo_us = 1000000 / amostras_por_segundo;

    // Função debounce para os botões
    bool debounce(uint pin) {
        const uint debounce_time = 50;
        if (!gpio_get(pin)) {
            sleep_ms(debounce_time);
            if (!gpio_get(pin)) {
                while (!gpio_get(pin)) {
                    sleep_ms(debounce_time);
                }
                return true;
            }
        }
        return false;
    }

    // Loop principal
    while (1)
    {
        // Lê o valor do ADC (microfone)
        adc_select_input(2);
        uint16_t leitura = adc_read();

        // Controle dos LEDs com base no som
        if (leitura > limiar_2)
        {
            // Som distante -> LED verde
            gpio_put(led_pin_green, 1);
            gpio_put(led_pin_red, 0);
            printf("Som distante\n");
        }
        else if (leitura > limiar_1)
        {
            // Som moderado -> LED vermelho
            gpio_put(led_pin_green, 0);
            gpio_put(led_pin_red, 1);
            printf("Som moderado\n");
        }
        else
        {
            // Sem som ou som baixo -> LEDs apagados
            gpio_put(led_pin_green, 0);
            gpio_put(led_pin_red, 0);
            printf("Sem som ou som baixo\n");
        }

        // Leitura do sensor de temperatura (canal 4)
        uint16_t raw_temp_value = adc_read();
        float voltage = raw_temp_value * 3.3f / (1 << 12);
        float referencia = 0.5928f;
        float coeficiente = 0.0021f;
        float temperatura_celsius = 27.0f - (voltage - referencia) / coeficiente;

        // Exibe os valores no Serial Monitor
        printf("Leitura Microfone: %d\n", leitura);
        if (leitura > limiar_2)
        {
            printf("Som distante (Verde)\n");
        }
        else if (leitura > limiar_1)
        {
            printf("Som moderado (Vermelho)\n");
        }
        else
                // Exibe a temperatura
                printf("Temperatura interna: %.2f °C\n", temperatura_celsius);

                // Verifica os botões e ajusta as ações (com debounce)
                if (debounce(BTN_A_PIN))
                {
                    matriz_ativa = true; // Liga a matriz de LEDs
                    printf("Matriz de LEDs ativada.\n");
                    numero_mostrado = (numero_mostrado + 1) % 10;
                }
                if (debounce(BTN_B_PIN))
                {
                    matriz_ativa = false; // Desliga a matriz de LEDs
                    printf("Matriz de LEDs desativada.\n");
                }
        
                // Controla a matriz de LEDs
                if (matriz_ativa)
                {
                    for (int i = 0; i < NUM_PIXELS; i++)
                    {
                        if (led_buffers[numero_mostrado][i])
                        {
                            pio_sm_put_blocking(pio, sm, 0xFFFFFF); // Liga o LED
                        }
                        else
                        {
                            pio_sm_put_blocking(pio, sm, 0); // Apaga o LED
                        }
                    }
                }
                else
                {
                    // Desliga a matriz de LEDs
                    for (int i = 0; i < NUM_PIXELS; i++)
                    {
                        pio_sm_put_blocking(pio, sm, 0); // Apaga todos os LEDs
                    }
                }
        
                // Delay para a frequência de amostragem do microfone
                sleep_us(intervalo_us);
            }
        
            return 0;
        }