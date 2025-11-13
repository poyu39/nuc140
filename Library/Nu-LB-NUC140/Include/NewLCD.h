#define LCD_Xmax 128
#define LCD_Ymax 64
#define FG_COLOR 0xFFFF
#define BG_COLOR 0x0000

extern void show_lcd_buffer(void);

extern void clear_lcd_buffer(void);

extern void cover_lcd_buffer(uint8_t input[]);

extern void clear_lcd(void);

extern void init_lcd(uint8_t auto_clear, uint32_t spi_clock_frequency);

extern void draw_bitmap_in_buffer(uint8_t bitmap[], int16_t x, int16_t y, int16_t bitmap_x_size, int16_t bitmap_y_size, uint16_t color);

extern void draw_pixel_in_buffer(int16_t x, int16_t y, uint16_t color);

extern void draw_line_in_buffer(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color);

extern void draw_circle_in_buffer(int16_t xc, int16_t yc, int16_t r, uint16_t color, uint8_t is_fill);

extern void draw_rectangle_in_buffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color, uint8_t is_fill);

extern void draw_triangle_in_buffer(int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color, uint8_t is_fill);

extern void print_c_in_buffer(int16_t x, int16_t y, uint8_t size, unsigned char ascii_code, uint16_t color);

extern void printf_s_in_buffer(int16_t x, int16_t y, uint8_t size, const char *format, ...);

extern void printf_line_in_buffer(int8_t line, uint8_t size, const char *format, ...);

extern uint8_t get_lcd_buffer_pixel(int16_t x, int16_t y);
