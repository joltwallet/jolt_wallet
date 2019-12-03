/* All of the functions available to an app.
 *
 * DO NOT CHANGE THIS FILE (jolt_lib.c) directly, either change:
 *     * elf2jelf/jolt_lib_template.c
 *     * elf2jelf/export_list.txt 
 *
 * jolt_lib.c will automatically be updated upon make
 * */

#include "jolt_lib.h"
#include "sdkconfig.h"

#if JOLT_OS

const jolt_version_t JOLT_JELF_VERSION = {
    .major = 0,
    .minor = 1,
    .patch = 0,
    .release = JOLT_VERSION_DEV,
};


extern int64_t __absvdi2(int64_t x);
extern int __absvsi2(int x);
extern double __adddf3(double x, double y);
extern float __addsf3(float x, float y);
extern int64_t __addvdi3(int64_t x, int64_t y);
extern int __addvsi3(int x, int y);
extern int64_t __ashldi3(int64_t x, int y);
extern int64_t __ashrdi3(int64_t x, int y);
extern int64_t __bswapdi2(int64_t x);
extern int32_t __bswapsi2(int32_t x);
extern int64_t __clrsbdi2(int64_t x);
extern int __clrsbsi2(int x);
extern int __clzdi2(int64_t x);
extern int __clzsi2(int x);
extern int __cmpdi2(int64_t x, int64_t y);
extern int __ctzdi2(uint64_t x);
extern int __ctzsi2(unsigned x);
extern complex double __divdc3(double a, double b, double c, double d);
extern double __divdf3(double x, double y);
extern int64_t __divdi3(int64_t x, int64_t y);
extern complex float __divsc3(float a, float b, float c, float d);
extern float __divsf3(float x, float y);
extern int __divsi3(int x, int y);
extern int __eqdf2(double x, double y);
extern int __eqsf2(float x, float y);
extern double __extendsfdf2(float x);
extern int __ffsdi2(uint64_t x);
extern int __ffssi2(unsigned x);
extern int64_t __fixdfdi(double x);
extern int __fixdfsi(double x);
extern int64_t __fixsfdi(float x);
extern int __fixsfsi(float x);
extern unsigned __fixunsdfsi(double x);
extern uint64_t __fixunssfdi(float x);
extern unsigned __fixunssfsi(float x);
extern double __floatdidf(int64_t);
extern float __floatdisf(int64_t);
extern double __floatsidf(int x);
extern float __floatsisf(int x);
extern double __floatundidf(uint64_t x);
extern float __floatundisf(uint64_t x);
extern double __floatunsidf(unsigned x);
extern float __floatunsisf(unsigned x);
extern int __gedf2(double x, double y);
extern int __gesf2(float x, float y);
extern int __gtdf2(double x, double y);
extern int __gtsf2(float x, float y);
extern int __ledf2(double x, double y);
extern int __lesf2(float x, float y);
extern int64_t __lshrdi3(int64_t x, int y);
extern int __ltdf2(double x, double y);
extern int __ltsf2(float x, float y);
extern int64_t __moddi3(int64_t x, int64_t y);
extern int __modsi3(int x, int y);
extern complex double __muldc3(double a, double b, double c, double d);
extern double __muldf3(double x, double y);
extern int64_t __muldi3(int64_t x, int64_t y);
extern complex float __mulsc3 (float a, float b, float c, float d);
extern float __mulsf3 (float a, float b);
extern int __mulsi3(int x, int y);
extern int __mulvdi3(int64_t x, int64_t y);
extern int __mulvsi3(int x, int y);
extern int __nedf2(double x, double y);
extern double __negdf2(double x);
extern int64_t __negdi2(int64_t x);
extern float __negsf2(float x);
extern int64_t __negvdi2(int64_t x);
extern int __negvsi2(int x);
extern int __nesf2(float x, float y);
extern int __paritysi2(unsigned x);
extern int __popcountdi2(uint64_t);
extern int __popcountsi2(unsigned x);
extern double __powidf2(double x, int y);
extern float __powisf2(float x, int y);
extern double __subdf3(double x, double y);
extern float __subsf3(float x, float y);
extern int64_t __subvdi3(int64_t x, int64_t y);
extern int __subvsi3(int x, int y);
extern float __truncdfsf2(double x);
extern int __ucmpdi2(uint64_t x, uint64_t y);
extern uint64_t __udivdi3(uint64_t x, uint64_t y);
extern uint64_t __udivmoddi4(uint64_t x, uint64_t y, uint64_t* z);
extern unsigned __udivsi3(unsigned x, unsigned y);
extern uint64_t __umoddi3(uint64_t x, uint64_t y);
extern unsigned __umodsi3(unsigned x, unsigned y);
extern uint64_t __umulsidi3(unsigned x, unsigned y);
extern int __unorddf2(double x, double y);
extern int __unordsf2(float x, float y);

#if CONFIG_COMPILER_STACK_CHECK
extern void *__stack_chk_fail;
extern void *__stack_chk_guard;
#else
static inline void __stack_chk_fail (void) { return; }
void *__stack_chk_guard = NULL;
#endif

#define EXPORT_SYMBOL(x) &x

/**
 * @brief Export functions to be used in applications.
 *
 * This order is very important; only *append* fuctions
 */
static const void *exports[] = {
    EXPORT_SYMBOL( __absvdi2 ),
    EXPORT_SYMBOL( __absvsi2 ),
    EXPORT_SYMBOL( __adddf3 ),
    EXPORT_SYMBOL( __addsf3 ),
    EXPORT_SYMBOL( __addvdi3 ),
    EXPORT_SYMBOL( __addvsi3 ),
    EXPORT_SYMBOL( __ashldi3 ),
    EXPORT_SYMBOL( __ashrdi3 ),
    EXPORT_SYMBOL( __bswapdi2 ),
    EXPORT_SYMBOL( __bswapsi2 ),
    EXPORT_SYMBOL( __clrsbdi2 ),
    EXPORT_SYMBOL( __clrsbsi2 ),
    EXPORT_SYMBOL( __clzdi2 ),
    EXPORT_SYMBOL( __clzsi2 ),
    EXPORT_SYMBOL( __cmpdi2 ),
    EXPORT_SYMBOL( __ctzdi2 ),
    EXPORT_SYMBOL( __ctzsi2 ),
    EXPORT_SYMBOL( __divdc3 ),
    EXPORT_SYMBOL( __divdf3 ),
    EXPORT_SYMBOL( __divdi3 ),
    EXPORT_SYMBOL( __divsi3 ),
    EXPORT_SYMBOL( __eqdf2 ),
    EXPORT_SYMBOL( __eqsf2 ),
    EXPORT_SYMBOL( __errno ),
    EXPORT_SYMBOL( __extendsfdf2 ),
    EXPORT_SYMBOL( __ffsdi2 ),
    EXPORT_SYMBOL( __ffssi2 ),
    EXPORT_SYMBOL( __fixdfdi ),
    EXPORT_SYMBOL( __fixdfsi ),
    EXPORT_SYMBOL( __fixsfdi ),
    EXPORT_SYMBOL( __fixsfsi ),
    EXPORT_SYMBOL( __fixunsdfsi ),
    EXPORT_SYMBOL( __fixunssfdi ),
    EXPORT_SYMBOL( __fixunssfsi ),
    EXPORT_SYMBOL( __floatdidf ),
    EXPORT_SYMBOL( __floatdisf ),
    EXPORT_SYMBOL( __floatsidf ),
    EXPORT_SYMBOL( __floatsisf ),
    EXPORT_SYMBOL( __floatundidf ),
    EXPORT_SYMBOL( __floatundisf ),
    EXPORT_SYMBOL( __floatunsidf ),
    EXPORT_SYMBOL( __floatunsisf ),
    EXPORT_SYMBOL( __gedf2 ),
    EXPORT_SYMBOL( __gesf2 ),
    EXPORT_SYMBOL( __gtdf2 ),
    EXPORT_SYMBOL( __gtsf2 ),
    EXPORT_SYMBOL( __ledf2 ),
    EXPORT_SYMBOL( __lesf2 ),
    EXPORT_SYMBOL( __lshrdi3 ),
    EXPORT_SYMBOL( __ltdf2 ),
    EXPORT_SYMBOL( __ltsf2 ),
    EXPORT_SYMBOL( __moddi3 ),
    EXPORT_SYMBOL( __modsi3 ),
    EXPORT_SYMBOL( __muldc3 ),
    EXPORT_SYMBOL( __muldf3 ),
    EXPORT_SYMBOL( __muldi3 ),
    EXPORT_SYMBOL( __mulsf3 ),
    EXPORT_SYMBOL( __mulsi3 ),
    EXPORT_SYMBOL( __mulvdi3 ),
    EXPORT_SYMBOL( __mulvsi3 ),
    EXPORT_SYMBOL( __nedf2 ),
    EXPORT_SYMBOL( __negdf2 ),
    EXPORT_SYMBOL( __negdi2 ),
    EXPORT_SYMBOL( __negsf2 ),
    EXPORT_SYMBOL( __negvdi2 ),
    EXPORT_SYMBOL( __negvsi2 ),
    EXPORT_SYMBOL( __nesf2 ),
    EXPORT_SYMBOL( __paritysi2 ),
    EXPORT_SYMBOL( __popcountdi2 ),
    EXPORT_SYMBOL( __popcountsi2 ),
    EXPORT_SYMBOL( __powidf2 ),
    EXPORT_SYMBOL( __subdf3 ),
    EXPORT_SYMBOL( __subsf3 ),
    EXPORT_SYMBOL( __subvdi3 ),
    EXPORT_SYMBOL( __subvsi3 ),
    EXPORT_SYMBOL( __truncdfsf2 ),
    EXPORT_SYMBOL( __ucmpdi2 ),
    EXPORT_SYMBOL( __udivdi3 ),
    EXPORT_SYMBOL( __udivmoddi4 ),
    EXPORT_SYMBOL( __udivsi3 ),
    EXPORT_SYMBOL( __umoddi3 ),
    EXPORT_SYMBOL( __umodsi3 ),
    EXPORT_SYMBOL( __umulsidi3 ),
    EXPORT_SYMBOL( __unorddf2 ),
    EXPORT_SYMBOL( __unordsf2 ),
    EXPORT_SYMBOL( _ctype_ ),
    EXPORT_SYMBOL( _daylight ),
    EXPORT_SYMBOL( environ ),
    EXPORT_SYMBOL( _global_impure_ptr ),
    EXPORT_SYMBOL( __sf_fake_stderr ),
    EXPORT_SYMBOL( __sf_fake_stdin ),
    EXPORT_SYMBOL( __sf_fake_stdout ),
    EXPORT_SYMBOL( _timezone ),
    EXPORT_SYMBOL( _tzname ),
    EXPORT_SYMBOL( abs ),
    EXPORT_SYMBOL( asctime ),
    EXPORT_SYMBOL( asctime_r ),
    EXPORT_SYMBOL( atoi ),
    EXPORT_SYMBOL( _atoi_r ),
    EXPORT_SYMBOL( atol ),
    EXPORT_SYMBOL( _atol_r ),
    EXPORT_SYMBOL( bzero ),
    EXPORT_SYMBOL( ctime ),
    EXPORT_SYMBOL( ctime_r ),
    EXPORT_SYMBOL( div ),
    EXPORT_SYMBOL( fclose ),
    EXPORT_SYMBOL( _fclose_r ),
    EXPORT_SYMBOL( fflush ),
    EXPORT_SYMBOL( _fflush_r ),
    EXPORT_SYMBOL( _findenv_r ),
    EXPORT_SYMBOL( fputwc ),
    EXPORT_SYMBOL( _fputwc_r ),
    EXPORT_SYMBOL( _getenv_r ),
    EXPORT_SYMBOL( __gettzinfo ),
    EXPORT_SYMBOL( gmtime ),
    EXPORT_SYMBOL( gmtime_r ),
    EXPORT_SYMBOL( isalnum ),
    EXPORT_SYMBOL( isalpha ),
    EXPORT_SYMBOL( isascii ),
    EXPORT_SYMBOL( isblank ),
    EXPORT_SYMBOL( iscntrl ),
    EXPORT_SYMBOL( isdigit ),
    EXPORT_SYMBOL( isgraph ),
    EXPORT_SYMBOL( islower ),
    EXPORT_SYMBOL( isprint ),
    EXPORT_SYMBOL( ispunct ),
    EXPORT_SYMBOL( isspace ),
    EXPORT_SYMBOL( isupper ),
    EXPORT_SYMBOL( __itoa ),
    EXPORT_SYMBOL( itoa ),
    EXPORT_SYMBOL( labs ),
    EXPORT_SYMBOL( ldiv ),
    EXPORT_SYMBOL( localtime ),
    EXPORT_SYMBOL( localtime_r ),
    EXPORT_SYMBOL( longjmp ),
    EXPORT_SYMBOL( memccpy ),
    EXPORT_SYMBOL( memchr ),
    EXPORT_SYMBOL( memcmp ),
    EXPORT_SYMBOL( memcpy ),
    EXPORT_SYMBOL( memmove ),
    EXPORT_SYMBOL( memrchr ),
    EXPORT_SYMBOL( memset ),
    EXPORT_SYMBOL( mktime ),
    EXPORT_SYMBOL( qsort ),
    EXPORT_SYMBOL( rand ),
    EXPORT_SYMBOL( rand_r ),
    EXPORT_SYMBOL( setjmp ),
    EXPORT_SYMBOL( __sinit ),
    EXPORT_SYMBOL( srand ),
    EXPORT_SYMBOL( strcasecmp ),
    EXPORT_SYMBOL( strcasestr ),
    EXPORT_SYMBOL( strcat ),
    EXPORT_SYMBOL( strchr ),
    EXPORT_SYMBOL( strcmp ),
    EXPORT_SYMBOL( strcoll ),
    EXPORT_SYMBOL( strcpy ),
    EXPORT_SYMBOL( strcspn ),
    EXPORT_SYMBOL( strdup ),
    EXPORT_SYMBOL( _strdup_r ),
    EXPORT_SYMBOL( strftime ),
    EXPORT_SYMBOL( strlcat ),
    EXPORT_SYMBOL( strlcpy ),
    EXPORT_SYMBOL( strlen ),
    EXPORT_SYMBOL( strlwr ),
    EXPORT_SYMBOL( strncasecmp ),
    EXPORT_SYMBOL( strncat ),
    EXPORT_SYMBOL( strncmp ),
    EXPORT_SYMBOL( strncpy ),
    EXPORT_SYMBOL( strndup ),
    EXPORT_SYMBOL( _strndup_r ),
    EXPORT_SYMBOL( strnlen ),
    EXPORT_SYMBOL( strrchr ),
    EXPORT_SYMBOL( strsep ),
    EXPORT_SYMBOL( strspn ),
    EXPORT_SYMBOL( strstr ),
    EXPORT_SYMBOL( strtok_r ),
    EXPORT_SYMBOL( strtol ),
    EXPORT_SYMBOL( _strtol_r ),
    EXPORT_SYMBOL( strtoul ),
    EXPORT_SYMBOL( _strtoul_r ),
    EXPORT_SYMBOL( strupr ),
    EXPORT_SYMBOL( __swbuf_r ),
    EXPORT_SYMBOL( time ),
    EXPORT_SYMBOL( toascii ),
    EXPORT_SYMBOL( tolower ),
    EXPORT_SYMBOL( toupper ),
    EXPORT_SYMBOL( tzset ),
    EXPORT_SYMBOL( _tzset_r ),
    EXPORT_SYMBOL( ungetc ),
    EXPORT_SYMBOL( _ungetc_r ),
    EXPORT_SYMBOL( __utoa ),
    EXPORT_SYMBOL( utoa ),
    EXPORT_SYMBOL( wcrtomb ),
    EXPORT_SYMBOL( _wcrtomb_r ),
    EXPORT_SYMBOL( _wctomb_r ),
    EXPORT_SYMBOL( localeconv ),
    EXPORT_SYMBOL( _localeconv_r ),
    EXPORT_SYMBOL( setlocale ),
    EXPORT_SYMBOL( _setlocale_r ),
    EXPORT_SYMBOL( asiprintf ),
    EXPORT_SYMBOL( _asiprintf_r ),
    EXPORT_SYMBOL( asniprintf ),
    EXPORT_SYMBOL( _asniprintf_r ),
    EXPORT_SYMBOL( asnprintf ),
    EXPORT_SYMBOL( _asnprintf_r ),
    EXPORT_SYMBOL( asprintf ),
    EXPORT_SYMBOL( _asprintf_r ),
    EXPORT_SYMBOL( fiprintf ),
    EXPORT_SYMBOL( _fiprintf_r ),
    EXPORT_SYMBOL( fiscanf ),
    EXPORT_SYMBOL( _fiscanf_r ),
    EXPORT_SYMBOL( fprintf ),
    EXPORT_SYMBOL( _fprintf_r ),
    EXPORT_SYMBOL( fscanf ),
    EXPORT_SYMBOL( _fscanf_r ),
    EXPORT_SYMBOL( iprintf ),
    EXPORT_SYMBOL( _iprintf_r ),
    EXPORT_SYMBOL( iscanf ),
    EXPORT_SYMBOL( _iscanf_r ),
    EXPORT_SYMBOL( printf ),
    EXPORT_SYMBOL( _printf_r ),
    EXPORT_SYMBOL( scanf ),
    EXPORT_SYMBOL( _scanf_r ),
    EXPORT_SYMBOL( siprintf ),
    EXPORT_SYMBOL( _siprintf_r ),
    EXPORT_SYMBOL( siscanf ),
    EXPORT_SYMBOL( _siscanf_r ),
    EXPORT_SYMBOL( sniprintf ),
    EXPORT_SYMBOL( _sniprintf_r ),
    EXPORT_SYMBOL( snprintf ),
    EXPORT_SYMBOL( _snprintf_r ),
    EXPORT_SYMBOL( sprintf ),
    EXPORT_SYMBOL( _sprintf_r ),
    EXPORT_SYMBOL( sscanf ),
    EXPORT_SYMBOL( _sscanf_r ),
    EXPORT_SYMBOL( vasiprintf ),
    EXPORT_SYMBOL( _vasiprintf_r ),
    EXPORT_SYMBOL( vasniprintf ),
    EXPORT_SYMBOL( _vasniprintf_r ),
    EXPORT_SYMBOL( vasnprintf ),
    EXPORT_SYMBOL( _vasnprintf_r ),
    EXPORT_SYMBOL( vasprintf ),
    EXPORT_SYMBOL( _vasprintf_r ),
    EXPORT_SYMBOL( vfiprintf ),
    EXPORT_SYMBOL( _vfiprintf_r ),
    EXPORT_SYMBOL( vfiscanf ),
    EXPORT_SYMBOL( _vfiscanf_r ),
    EXPORT_SYMBOL( vfprintf ),
    EXPORT_SYMBOL( _vfprintf_r ),
    EXPORT_SYMBOL( vfscanf ),
    EXPORT_SYMBOL( _vfscanf_r ),
    EXPORT_SYMBOL( viprintf ),
    EXPORT_SYMBOL( _viprintf_r ),
    EXPORT_SYMBOL( viscanf ),
    EXPORT_SYMBOL( _viscanf_r ),
    EXPORT_SYMBOL( vprintf ),
    EXPORT_SYMBOL( _vprintf_r ),
    EXPORT_SYMBOL( vscanf ),
    EXPORT_SYMBOL( _vscanf_r ),
    EXPORT_SYMBOL( vsiprintf ),
    EXPORT_SYMBOL( _vsiprintf_r ),
    EXPORT_SYMBOL( vsiscanf ),
    EXPORT_SYMBOL( _vsiscanf_r ),
    EXPORT_SYMBOL( vsniprintf ),
    EXPORT_SYMBOL( _vsniprintf_r ),
    EXPORT_SYMBOL( vsnprintf ),
    EXPORT_SYMBOL( _vsnprintf_r ),
    EXPORT_SYMBOL( vsprintf ),
    EXPORT_SYMBOL( _vsprintf_r ),
    EXPORT_SYMBOL( vsscanf ),
    EXPORT_SYMBOL( _vsscanf_r ),
    EXPORT_SYMBOL( __stack_chk_fail ),
    EXPORT_SYMBOL( __stack_chk_guard ),
    EXPORT_SYMBOL( abort ),
    EXPORT_SYMBOL( calloc ),
    EXPORT_SYMBOL( cJSON_AddArrayToObject ),
    EXPORT_SYMBOL( cJSON_AddBoolToObject ),
    EXPORT_SYMBOL( cJSON_AddFalseToObject ),
    EXPORT_SYMBOL( cJSON_AddItemReferenceToArray ),
    EXPORT_SYMBOL( cJSON_AddItemReferenceToObject ),
    EXPORT_SYMBOL( cJSON_AddItemToArray ),
    EXPORT_SYMBOL( cJSON_AddItemToObject ),
    EXPORT_SYMBOL( cJSON_AddItemToObjectCS ),
    EXPORT_SYMBOL( cJSON_AddNullToObject ),
    EXPORT_SYMBOL( cJSON_AddNumberToObject ),
    EXPORT_SYMBOL( cJSON_AddObjectToObject ),
    EXPORT_SYMBOL( cJSON_AddRawToObject ),
    EXPORT_SYMBOL( cJSON_AddStringToObject ),
    EXPORT_SYMBOL( cJSON_AddTrueToObject ),
    EXPORT_SYMBOL( cJSON_CreateArray ),
    EXPORT_SYMBOL( cJSON_CreateArrayReference ),
    EXPORT_SYMBOL( cJSON_CreateBool ),
    EXPORT_SYMBOL( cJSON_CreateDoubleArray ),
    EXPORT_SYMBOL( cJSON_CreateFalse ),
    EXPORT_SYMBOL( cJSON_CreateFloatArray ),
    EXPORT_SYMBOL( cJSON_CreateIntArray ),
    EXPORT_SYMBOL( cJSON_CreateNull ),
    EXPORT_SYMBOL( cJSON_CreateNumber ),
    EXPORT_SYMBOL( cJSON_CreateObject ),
    EXPORT_SYMBOL( cJSON_CreateObjectReference ),
    EXPORT_SYMBOL( cJSON_CreateRaw ),
    EXPORT_SYMBOL( cJSON_CreateString ),
    EXPORT_SYMBOL( cJSON_CreateStringArray ),
    EXPORT_SYMBOL( cJSON_CreateStringReference ),
    EXPORT_SYMBOL( cJSON_CreateTrue ),
    EXPORT_SYMBOL( cJSON_Delete ),
    EXPORT_SYMBOL( cJSON_DeleteItemFromArray ),
    EXPORT_SYMBOL( cJSON_DeleteItemFromObject ),
    EXPORT_SYMBOL( cJSON_DeleteItemFromObjectCaseSensitive ),
    EXPORT_SYMBOL( cJSON_DetachItemFromArray ),
    EXPORT_SYMBOL( cJSON_DetachItemFromObject ),
    EXPORT_SYMBOL( cJSON_DetachItemFromObjectCaseSensitive ),
    EXPORT_SYMBOL( cJSON_DetachItemViaPointer ),
    EXPORT_SYMBOL( cJSON_GetArrayItem ),
    EXPORT_SYMBOL( cJSON_GetArraySize ),
    EXPORT_SYMBOL( cJSON_GetObjectItem ),
    EXPORT_SYMBOL( cJSON_GetObjectItemCaseSensitive ),
    EXPORT_SYMBOL( cJSON_GetStringValue ),
    EXPORT_SYMBOL( cJSON_InsertItemInArray ),
    EXPORT_SYMBOL( cJSON_IsArray ),
    EXPORT_SYMBOL( cJSON_IsBool ),
    EXPORT_SYMBOL( cJSON_IsFalse ),
    EXPORT_SYMBOL( cJSON_IsInvalid ),
    EXPORT_SYMBOL( cJSON_IsNull ),
    EXPORT_SYMBOL( cJSON_IsNumber ),
    EXPORT_SYMBOL( cJSON_IsObject ),
    EXPORT_SYMBOL( cJSON_IsRaw ),
    EXPORT_SYMBOL( cJSON_IsString ),
    EXPORT_SYMBOL( cJSON_IsTrue ),
    EXPORT_SYMBOL( cJSON_Parse ),
    EXPORT_SYMBOL( cJSON_Print ),
    EXPORT_SYMBOL( cJSON_PrintUnformatted ),
    EXPORT_SYMBOL( cJSON_ReplaceItemInArray ),
    EXPORT_SYMBOL( cJSON_ReplaceItemInObject ),
    EXPORT_SYMBOL( cJSON_ReplaceItemInObjectCaseSensitive ),
    EXPORT_SYMBOL( cJSON_ReplaceItemViaPointer ),
    EXPORT_SYMBOL( cJSON_SetNumberHelper ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_frombytes_negate_vartime ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_double_scalarmult_vartime ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_tobytes ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_sc_muladd ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_scalarmult_base ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_sc_reduce ),
    EXPORT_SYMBOL( crypto_core_curve25519_ref10_ge_p3_tobytes ),
    EXPORT_SYMBOL( console_check_equal_argc ),
    EXPORT_SYMBOL( console_check_range_argc ),
    EXPORT_SYMBOL( esp_console_deinit ),
    EXPORT_SYMBOL( esp_console_init ),
    EXPORT_SYMBOL( esp_err_to_name ),
    EXPORT_SYMBOL( esp_log_timestamp ),
    EXPORT_SYMBOL( esp_log_write ),
    EXPORT_SYMBOL( esp_restart ),
    EXPORT_SYMBOL( free ),
    EXPORT_SYMBOL( hd_node_copy ),
    EXPORT_SYMBOL( hd_node_iterate ),
    EXPORT_SYMBOL( heap_caps_calloc ),
    EXPORT_SYMBOL( heap_caps_check_integrity_all ),
    EXPORT_SYMBOL( jolt_app_cmd_contact ),
    EXPORT_SYMBOL( jolt_cli_get_line ),
    EXPORT_SYMBOL( jolt_cli_return ),
    EXPORT_SYMBOL( jolt_cli_sub_cmd_free ),
    EXPORT_SYMBOL( jolt_cli_sub_cmd_register ),
    EXPORT_SYMBOL( jolt_cli_sub_cmd_run ),
    EXPORT_SYMBOL( jolt_cli_sub_init ),
    EXPORT_SYMBOL( jolt_crypto_derive ),
    EXPORT_SYMBOL( jolt_crypto_from_str ),
    EXPORT_SYMBOL( jolt_crypto_sign ),
    EXPORT_SYMBOL( jolt_crypto_status_to_str ),
    EXPORT_SYMBOL( jolt_crypto_verify ),
    EXPORT_SYMBOL( jolt_display_get_brightness ),
    EXPORT_SYMBOL( jolt_display_set_brightness ),
    EXPORT_SYMBOL( jolt_gui_err_to_str ),
    EXPORT_SYMBOL( jolt_gui_event ),
    EXPORT_SYMBOL( jolt_gui_event_del ),
    EXPORT_SYMBOL( jolt_gui_find ),
    EXPORT_SYMBOL( jolt_gui_group_add ),
    EXPORT_SYMBOL( jolt_gui_obj_cont_body_create ),
    EXPORT_SYMBOL( jolt_gui_obj_del ),
    EXPORT_SYMBOL( jolt_gui_obj_digit_entry_get_arr ),
    EXPORT_SYMBOL( jolt_gui_obj_digit_entry_get_double ),
    EXPORT_SYMBOL( jolt_gui_obj_digit_entry_get_hash ),
    EXPORT_SYMBOL( jolt_gui_obj_digit_entry_get_int ),
    EXPORT_SYMBOL( jolt_gui_obj_get_param ),
    EXPORT_SYMBOL( jolt_gui_obj_get_parent ),
    EXPORT_SYMBOL( jolt_gui_obj_id_get ),
    EXPORT_SYMBOL( jolt_gui_obj_parent_create ),
    EXPORT_SYMBOL( jolt_gui_obj_set_event_cb ),
    EXPORT_SYMBOL( jolt_gui_obj_set_param ),
    EXPORT_SYMBOL( jolt_gui_obj_title_create ),
    EXPORT_SYMBOL( jolt_gui_scr_bignum_create ),
    EXPORT_SYMBOL( jolt_gui_scr_del ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_create ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_get_arr ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_get_double ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_get_hash ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_get_int ),
    EXPORT_SYMBOL( jolt_gui_scr_digit_entry_set_pos ),
    EXPORT_SYMBOL( jolt_gui_scr_err_create ),
    EXPORT_SYMBOL( jolt_gui_scr_get ),
    EXPORT_SYMBOL( jolt_gui_scr_get_active ),
    EXPORT_SYMBOL( jolt_gui_scr_id_get ),
    EXPORT_SYMBOL( jolt_gui_scr_loadingbar_autoupdate ),
    EXPORT_SYMBOL( jolt_gui_scr_loadingbar_create ),
    EXPORT_SYMBOL( jolt_gui_scr_loadingbar_progress_get ),
    EXPORT_SYMBOL( jolt_gui_scr_loadingbar_update ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_add ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_add_info ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_add_sw ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_create ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_get_btn_index ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_get_list ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_get_scr ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_len ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_remove ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_remove_indices ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_set_btn_selected ),
    EXPORT_SYMBOL( jolt_gui_scr_menu_set_param ),
    EXPORT_SYMBOL( jolt_gui_scr_preloading_create ),
    EXPORT_SYMBOL( jolt_gui_scr_preloading_update ),
    EXPORT_SYMBOL( jolt_gui_scr_qr_create ),
    EXPORT_SYMBOL( jolt_gui_scr_scroll_add_monospace_text ),
    EXPORT_SYMBOL( jolt_gui_scr_scroll_add_qr ),
    EXPORT_SYMBOL( jolt_gui_scr_scroll_add_text ),
    EXPORT_SYMBOL( jolt_gui_scr_scroll_create ),
    EXPORT_SYMBOL( jolt_gui_scr_scroll_get_page ),
    EXPORT_SYMBOL( jolt_gui_scr_set_active_param ),
    EXPORT_SYMBOL( jolt_gui_scr_set_event_cb ),
    EXPORT_SYMBOL( jolt_gui_scr_slider_create ),
    EXPORT_SYMBOL( jolt_gui_scr_slider_get_value ),
    EXPORT_SYMBOL( jolt_gui_scr_slider_set_label ),
    EXPORT_SYMBOL( jolt_gui_scr_slider_set_range ),
    EXPORT_SYMBOL( jolt_gui_scr_slider_set_value ),
    EXPORT_SYMBOL( jolt_gui_scr_text_create ),
    EXPORT_SYMBOL( jolt_gui_scr_yesno_create ),
    EXPORT_SYMBOL( jolt_gui_sem_give ),
    EXPORT_SYMBOL( jolt_gui_sem_take ),
    EXPORT_SYMBOL( jolt_hash_final ),
    EXPORT_SYMBOL( jolt_hash_init ),
    EXPORT_SYMBOL( jolt_hash_update ),
    EXPORT_SYMBOL( jolt_json_del_app ),
    EXPORT_SYMBOL( jolt_json_read_app ),
    EXPORT_SYMBOL( jolt_json_write_app ),
    EXPORT_SYMBOL( jolt_launch_get_name ),
    EXPORT_SYMBOL( jolt_network_post ),
    EXPORT_SYMBOL( jolt_random ),
    EXPORT_SYMBOL( jolt_random_range ),
    EXPORT_SYMBOL( jolt_resume_logging ),
    EXPORT_SYMBOL( jolt_shuffle_arr ),
    EXPORT_SYMBOL( jolt_suspend_logging ),
    EXPORT_SYMBOL( malloc ),
    EXPORT_SYMBOL( mbedtls_mpi_add_abs ),
    EXPORT_SYMBOL( mbedtls_mpi_add_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_cmp_abs ),
    EXPORT_SYMBOL( mbedtls_mpi_cmp_int ),
    EXPORT_SYMBOL( mbedtls_mpi_cmp_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_copy ),
    EXPORT_SYMBOL( mbedtls_mpi_div_int ),
    EXPORT_SYMBOL( mbedtls_mpi_div_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_free ),
    EXPORT_SYMBOL( mbedtls_mpi_get_bit ),
    EXPORT_SYMBOL( mbedtls_mpi_init ),
    EXPORT_SYMBOL( mbedtls_mpi_lset ),
    EXPORT_SYMBOL( mbedtls_mpi_mod_int ),
    EXPORT_SYMBOL( mbedtls_mpi_mod_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_mul_int ),
    EXPORT_SYMBOL( mbedtls_mpi_mul_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_read_binary ),
    EXPORT_SYMBOL( mbedtls_mpi_read_string ),
    EXPORT_SYMBOL( mbedtls_mpi_set_bit ),
    EXPORT_SYMBOL( mbedtls_mpi_sub_abs ),
    EXPORT_SYMBOL( mbedtls_mpi_sub_int ),
    EXPORT_SYMBOL( mbedtls_mpi_sub_mpi ),
    EXPORT_SYMBOL( mbedtls_mpi_swap ),
    EXPORT_SYMBOL( mbedtls_mpi_write_binary ),
    EXPORT_SYMBOL( mbedtls_mpi_write_string ),
    EXPORT_SYMBOL( putchar ),
    EXPORT_SYMBOL( puts ),
    EXPORT_SYMBOL( qrcode_getBufferSize ),
    EXPORT_SYMBOL( qrcode_initText ),
    EXPORT_SYMBOL( sodium_bin2hex ),
    EXPORT_SYMBOL( sodium_hex2bin ),
    EXPORT_SYMBOL( sodium_malloc ),
    EXPORT_SYMBOL( sodium_memcmp ),
    EXPORT_SYMBOL( sodium_memzero ),
    EXPORT_SYMBOL( vault_get_node ),
    EXPORT_SYMBOL( vault_get_valid ),
    EXPORT_SYMBOL( vault_refresh ),
    EXPORT_SYMBOL( vault_sem_give ),
    EXPORT_SYMBOL( vault_sem_take ),
    EXPORT_SYMBOL( vault_str_to_purpose_type ),

};

#else

const jolt_version_t JOLT_JELF_VERSION = { 0 };

/* Dummy place holder */
static const void *exports[490] = { 0 };

#endif

const jelfLoaderEnv_t jelf_loader_env = {
    .exported = exports,
    .exported_size = sizeof(exports) / sizeof(*exports)
};
