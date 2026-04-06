void wifi_list_init();
void wifi_list_update(const char *value);
void wifi_list_draw(struct field *f);
void wifi_list_select(struct field *f, uint16_t tx, uint16_t ty);
void wifi_list_scroll(struct field *f, int direction);
