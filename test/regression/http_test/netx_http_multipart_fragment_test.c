/* This case tests whether HTTP server can work well with multipart/form-data.
 * server packet size 800
 * client packet size 1400
 * pkt size is bigger than 1500 to cause client/server fragmentation.
 */

/* Here is the packet content. */
/*
------------------------------4ebf00fbcf09
Content-Disposition: form-data; name="example"

testtest(repeat 49 times)
------------------------------4ebf00fbcf09
Content-Disposition: form-data; name="example2"

test2test2(repeat 35 times)
------------------------------4ebf00fbcf09
Content-Disposition: form-data; name="example3"

test3test3(repeat 39 times)
------------------------------4ebf00fbcf09
Content-Disposition: form-data; name="example4"

test4test4(repeat 51times)
------------------------------4ebf00fbcf09
Content-Disposition: form-data; name="example5"

test5(repeat 54times)
------------------------------4ebf00fbcf09--
*/

/* length1 = 196,
 * length2 = 175,
 * length3 = 195,
 * length4 = 205
 * length5 = 220.
 * */

#include    "tx_api.h"
#include    "nx_api.h"
#include    "fx_api.h"

extern void test_control_return(UINT);
#if defined(NX_HTTP_MULTIPART_ENABLE) && !defined(NX_DISABLE_IPV4)
#include    "nxd_http_client.h"
#include    "nxd_http_server.h"

#define     DEMO_STACK_SIZE         4096

/* This is a HTTP get packet captured by wireshark. HEAD index.html*/
static char pkt[] = {
    0x50, 0x4f, 0x53, 0x54, 0x20, 0x2f, 0x20, 0x48, /* POST / H */
    0x54, 0x54, 0x50, 0x2f, 0x31, 0x2e, 0x31, 0x0d, /* TTP/1.1. */
    0x0a, 0x55, 0x73, 0x65, 0x72, 0x2d, 0x41, 0x67, /* .User-Ag */
    0x65, 0x6e, 0x74, 0x3a, 0x20, 0x63, 0x75, 0x72, /* ent: cur */
    0x6c, 0x2f, 0x37, 0x2e, 0x33, 0x32, 0x2e, 0x30, /* l/7.32.0 */
    0x0d, 0x0a, 0x48, 0x6f, 0x73, 0x74, 0x3a, 0x20, /* ..Host:  */
    0x31, 0x39, 0x32, 0x2e, 0x31, 0x36, 0x38, 0x2e, /* 192.168. */
    0x30, 0x2e, 0x31, 0x32, 0x33, 0x0d, 0x0a, 0x41, /* 0.123..A */
    0x63, 0x63, 0x65, 0x70, 0x74, 0x3a, 0x20, 0x2a, /* ccept: * */
    0x2f, 0x2a, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x74, /*  *..Cont */
    0x65, 0x6e, 0x74, 0x2d, 0x54, 0x79, 0x70, 0x65, /* ent-Type */
    0x3a, 0x20, 0x6d, 0x75, 0x6c, 0x74, 0x69, 0x70, /* : multip */
    0x61, 0x72, 0x74, 0x2f, 0x66, 0x6f, 0x72, 0x6d, /* art/form */
    0x2d, 0x64, 0x61, 0x74, 0x61, 0x3b, 0x20, 0x62, /* -data; b */
    0x6f, 0x75, 0x6e, 0x64, 0x61, 0x72, 0x79, 0x3d, /* oundary= */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x34, 0x65, 0x62, 0x66, /* ----4ebf */
    0x30, 0x30, 0x66, 0x62, 0x63, 0x66, 0x30, 0x39, /* 00fbcf09 */
    0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, /* ..Conten */
    0x74, 0x2d, 0x4c, 0x65, 0x6e, 0x67, 0x74, 0x68, /* t-Length */
    0x3a, 0x20, 0x31, 0x35, 0x32, 0x31, 0x0d, 0x0a, /* : 1521.. */
    0x45, 0x78, 0x70, 0x65, 0x63, 0x74, 0x3a, 0x20, /* Expect:  */
    0x31, 0x30, 0x30, 0x2d, 0x63, 0x6f, 0x6e, 0x74, /* 100-cont */
    0x69, 0x6e, 0x75, 0x65, 0x0d, 0x0a, 0x0d, 0x0a, /* inue.... */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x34, 0x65, /* ------4e */
    0x62, 0x66, 0x30, 0x30, 0x66, 0x62, 0x63, 0x66, /* bf00fbcf */
    0x30, 0x39, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x74, /* 09..Cont */
    0x65, 0x6e, 0x74, 0x2d, 0x44, 0x69, 0x73, 0x70, /* ent-Disp */
    0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3a, /* osition: */
    0x20, 0x66, 0x6f, 0x72, 0x6d, 0x2d, 0x64, 0x61, /*  form-da */
    0x74, 0x61, 0x3b, 0x20, 0x6e, 0x61, 0x6d, 0x65, /* ta; name */
    0x3d, 0x22, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, /* ="exampl */
    0x65, 0x22, 0x0d, 0x0a, 0x0d, 0x0a, 0x74, 0x65, /* e"....te */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x74, 0x65, 0x73, 0x74, 0x74, 0x65, /* sttestte */
    0x73, 0x74, 0x0d, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, /* st..---- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x34, 0x65, 0x62, 0x66, 0x30, 0x30, /* --4ebf00 */
    0x66, 0x62, 0x63, 0x66, 0x30, 0x39, 0x0d, 0x0a, /* fbcf09.. */
    0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, /* Content- */
    0x44, 0x69, 0x73, 0x70, 0x6f, 0x73, 0x69, 0x74, /* Disposit */
    0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x66, 0x6f, 0x72, /* ion: for */
    0x6d, 0x2d, 0x64, 0x61, 0x74, 0x61, 0x3b, 0x20, /* m-data;  */
    0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x65, 0x78, /* name="ex */
    0x61, 0x6d, 0x70, 0x6c, 0x65, 0x32, 0x22, 0x0d, /* ample2". */
    0x0a, 0x0d, 0x0a, 0x74, 0x65, 0x73, 0x74, 0x32, /* ...test2 */
    0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, /* test2tes */
    0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, /* t2test2t */
    0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, /* est2test */
    0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, /* 2test2te */
    0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, /* st2test2 */
    0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, /* test2tes */
    0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, /* t2test2t */
    0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, /* est2test */
    0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, /* 2test2te */
    0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, /* st2test2 */
    0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, /* test2tes */
    0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, /* t2test2t */
    0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, /* est2test */
    0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, /* 2test2te */
    0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, /* st2test2 */
    0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, /* test2tes */
    0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, /* t2test2t */
    0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, /* est2test */
    0x32, 0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, /* 2test2te */
    0x73, 0x74, 0x32, 0x74, 0x65, 0x73, 0x74, 0x32, /* st2test2 */
    0x74, 0x65, 0x73, 0x74, 0x32, 0x74, 0x65, 0x73, /* test2tes */
    0x74, 0x32, 0x0d, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, /* t2..---- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x34, 0x65, 0x62, 0x66, 0x30, 0x30, /* --4ebf00 */
    0x66, 0x62, 0x63, 0x66, 0x30, 0x39, 0x0d, 0x0a, /* fbcf09.. */
    0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, 0x74, 0x2d, /* Content- */
    0x44, 0x69, 0x73, 0x70, 0x6f, 0x73, 0x69, 0x74, /* Disposit */
    0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x66, 0x6f, 0x72, /* ion: for */
    0x6d, 0x2d, 0x64, 0x61, 0x74, 0x61, 0x3b, 0x20, /* m-data;  */
    0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, 0x65, 0x78, /* name="ex */
    0x61, 0x6d, 0x70, 0x6c, 0x65, 0x33, 0x22, 0x0d, /* ample3". */
    0x0a, 0x0d, 0x0a, 0x74, 0x65, 0x73, 0x74, 0x33, /* ...test3 */
    0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, /* test3tes */
    0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, /* t3test3t */
    0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, /* est3test */
    0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, /* 3test3te */
    0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, /* st3test3 */
    0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, /* test3tes */
    0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, /* t3test3t */
    0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, /* est3test */
    0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, /* 3test3te */
    0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, /* st3test3 */
    0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, /* test3tes */
    0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, /* t3test3t */
    0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, /* est3test */
    0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, /* 3test3te */
    0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, /* st3test3 */
    0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, /* test3tes */
    0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, /* t3test3t */
    0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, /* est3test */
    0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, /* 3test3te */
    0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, /* st3test3 */
    0x74, 0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, /* test3tes */
    0x74, 0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x74, /* t3test3t */
    0x65, 0x73, 0x74, 0x33, 0x74, 0x65, 0x73, 0x74, /* est3test */
    0x33, 0x74, 0x65, 0x73, 0x74, 0x33, 0x0d, 0x0a, /* 3test3.. */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x34, 0x65, /* ------4e */
    0x62, 0x66, 0x30, 0x30, 0x66, 0x62, 0x63, 0x66, /* bf00fbcf */
    0x30, 0x39, 0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x74, /* 09..Cont */
    0x65, 0x6e, 0x74, 0x2d, 0x44, 0x69, 0x73, 0x70, /* ent-Disp */
    0x6f, 0x73, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3a, /* osition: */
    0x20, 0x66, 0x6f, 0x72, 0x6d, 0x2d, 0x64, 0x61, /*  form-da */
    0x74, 0x61, 0x3b, 0x20, 0x6e, 0x61, 0x6d, 0x65, /* ta; name */
    0x3d, 0x22, 0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, /* ="exampl */
    0x65, 0x34, 0x22, 0x0d, 0x0a, 0x0d, 0x0a, 0x74, /* e4"....t */
    0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, /* est4test */
    0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, /* 4test4te */
    0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, /* st4test4 */
    0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, /* test4tes */
    0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, /* t4test4t */
    0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, /* est4test */
    0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, /* 4test4te */
    0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, /* st4test4 */
    0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, /* test4tes */
    0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, /* t4test4t */
    0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, /* est4test */
    0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, /* 4test4te */
    0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, /* st4test4 */
    0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, /* test4tes */
    0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, /* t4test4t */
    0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, /* est4test */
    0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, /* 4test4te */
    0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, /* st4test4 */
    0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, /* test4tes */
    0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, /* t4test4t */
    0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, /* est4test */
    0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, /* 4test4te */
    0x73, 0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, /* st4test4 */
    0x74, 0x65, 0x73, 0x74, 0x34, 0x74, 0x65, 0x73, /* test4tes */
    0x74, 0x34, 0x74, 0x65, 0x73, 0x74, 0x34, 0x74, /* t4test4t */
    0x65, 0x73, 0x74, 0x34, 0x0d, 0x0a, 0x2d, 0x2d, /* est4..-- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x34, 0x65, 0x62, 0x66, /* ----4ebf */
    0x30, 0x30, 0x66, 0x62, 0x63, 0x66, 0x30, 0x39, /* 00fbcf09 */
    0x0d, 0x0a, 0x43, 0x6f, 0x6e, 0x74, 0x65, 0x6e, /* ..Conten */
    0x74, 0x2d, 0x44, 0x69, 0x73, 0x70, 0x6f, 0x73, /* t-Dispos */
    0x69, 0x74, 0x69, 0x6f, 0x6e, 0x3a, 0x20, 0x66, /* ition: f */
    0x6f, 0x72, 0x6d, 0x2d, 0x64, 0x61, 0x74, 0x61, /* orm-data */
    0x3b, 0x20, 0x6e, 0x61, 0x6d, 0x65, 0x3d, 0x22, /* ; name=" */
    0x65, 0x78, 0x61, 0x6d, 0x70, 0x6c, 0x65, 0x35, /* example5 */
    0x22, 0x0d, 0x0a, 0x0d, 0x0a, 0x74, 0x65, 0x73, /* "....tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, /* 5test5te */
    0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, /* st5test5 */
    0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, /* test5tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, /* 5test5te */
    0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, /* st5test5 */
    0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, /* test5tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, /* 5test5te */
    0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, /* st5test5 */
    0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, /* test5tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, /* 5test5te */
    0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, /* st5test5 */
    0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, /* test5tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, /* 5test5te */
    0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, /* st5test5 */
    0x74, 0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, /* test5tes */
    0x74, 0x35, 0x74, 0x65, 0x73, 0x74, 0x35, 0x74, /* t5test5t */
    0x65, 0x73, 0x74, 0x35, 0x74, 0x65, 0x73, 0x74, /* est5test */
    0x35, 0x0d, 0x0a, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* 5..----- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, 0x2d, /* -------- */
    0x2d, 0x34, 0x65, 0x62, 0x66, 0x30, 0x30, 0x66, /* -4ebf00f */
    0x62, 0x63, 0x66, 0x30, 0x39, 0x2d, 0x2d, 0x0d, /* bcf09--. */
    0x0a                                            /* . */
};


/* Set up FileX and file memory resources. */
static CHAR             *ram_disk_memory;
static FX_MEDIA         ram_disk;

/* Define device drivers.  */
extern void _fx_ram_driver(FX_MEDIA *media_ptr);
extern void _nx_ram_network_driver_1024(NX_IP_DRIVER *driver_req_ptr);

static UINT my_get_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr);

/* Set up the HTTP client global variables. */

#define CLIENT_PACKET_SIZE  1400

static TX_THREAD       client_thread;
static NX_PACKET_POOL  client_pool;
static NX_IP           client_ip;
static UINT            error_counter;

static NX_TCP_SOCKET   client_socket;

/* Set up the HTTP server global variables */

#define SERVER_PACKET_SIZE  800

static NX_HTTP_SERVER  my_server;
static NX_PACKET_POOL  server_pool;
static TX_THREAD       server_thread;
static NX_IP           server_ip;
#ifdef __PRODUCT_NETXDUO__
static NXD_ADDRESS     server_ip_address;
#else
static ULONG           server_ip_address;
#endif

static UINT multipart_flag; 

static void thread_client_entry(ULONG thread_input);
static void thread_server_entry(ULONG thread_input);

#define HTTP_SERVER_ADDRESS  IP_ADDRESS(192,168,0,105)
#define HTTP_CLIENT_ADDRESS  IP_ADDRESS(192,168,0,123)

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_http_multipart_fragment_test_application_define(void *first_unused_memory)
#endif
{

CHAR    *pointer;
UINT    status;

    error_counter = 0;

    /* Setup the working pointer.  */
    pointer =  (CHAR *) first_unused_memory;

    /* Create a helper thread for the server. */
    tx_thread_create(&server_thread, "HTTP Server thread", thread_server_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     4, 4, TX_NO_TIME_SLICE, TX_AUTO_START);

    pointer =  pointer + DEMO_STACK_SIZE;

    /* Initialize the NetX system.  */
    nx_system_initialize();

    /* Create the server packet pool.  */
    status =  nx_packet_pool_create(&server_pool, "HTTP Server Packet Pool", SERVER_PACKET_SIZE, 
                                    pointer, SERVER_PACKET_SIZE*8);
    pointer = pointer + SERVER_PACKET_SIZE * 8;
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&server_ip, "HTTP Server IP", HTTP_SERVER_ADDRESS, 
                          0xFFFFFF00UL, &server_pool, _nx_ram_network_driver_1024,
                          pointer, 4096, 1);
    pointer =  pointer + 4096;
    if (status)
        error_counter++;

    /* Enable ARP and supply ARP cache memory for the server IP instance.  */
    status = nx_arp_enable(&server_ip, (void *) pointer, 1024);
    pointer = pointer + 1024;
    if (status)
        error_counter++;

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&server_ip);
    if (status)
        error_counter++;

    /* Set up the server's IPv4 address here. */
#ifdef __PRODUCT_NETXDUO__ 
    server_ip_address.nxd_ip_address.v4 = HTTP_SERVER_ADDRESS;
    server_ip_address.nxd_ip_version = NX_IP_VERSION_V4;
#else
    server_ip_address = HTTP_SERVER_ADDRESS;
#endif

    /* Create the HTTP Server.  */
    status = nx_http_server_create(&my_server, "My HTTP Server", &server_ip, &ram_disk, 
                          pointer, 2048, &server_pool, NX_NULL, my_get_notify);
    pointer =  pointer + 2048;
    if (status)
        error_counter++;

    /* Save the memory pointer for the RAM disk.  */
    ram_disk_memory =  pointer;

    /* Create the HTTP Client thread. */
    status = tx_thread_create(&client_thread, "HTTP Client", thread_client_entry, 0,  
                     pointer, DEMO_STACK_SIZE, 
                     6, 6, TX_NO_TIME_SLICE, TX_AUTO_START);
    pointer =  pointer + DEMO_STACK_SIZE;
    if (status)
        error_counter++;

    /* Create the Client packet pool.  */
    status =  nx_packet_pool_create(&client_pool, "HTTP Client Packet Pool", CLIENT_PACKET_SIZE, 
                                    pointer, CLIENT_PACKET_SIZE*8);
    pointer = pointer + CLIENT_PACKET_SIZE * 8;
    if (status)
        error_counter++;

    /* Create an IP instance.  */
    status = nx_ip_create(&client_ip, "HTTP Client IP", HTTP_CLIENT_ADDRESS, 
                          0xFFFFFF00UL, &client_pool, _nx_ram_network_driver_1024,
                          pointer, 2048, 1);
    pointer =  pointer + 2048;
    if (status)
        error_counter++;

    status  = nx_arp_enable(&client_ip, (void *) pointer, 1024);
    pointer =  pointer + 2048;
    if (status)
        error_counter++;

     /* Enable TCP traffic.  */
    status = nx_tcp_enable(&client_ip);
    if (status)
        error_counter++;

}


void thread_client_entry(ULONG thread_input)
{

UINT            status;
NX_PACKET       *recv_packet;
NX_PACKET       *my_packet;
CHAR            *buffer_ptr;

    multipart_flag = 0;

    /* Create a socket.  */
    status = nx_tcp_socket_create(&client_ip, &client_socket, "Client Socket", 
                                  NX_IP_NORMAL, NX_FRAGMENT_OKAY, NX_IP_TIME_TO_LIVE, 1024,
                                  NX_NULL, NX_NULL);
    if(status)
        error_counter++;

    /* Bind the socket.  */
    status = nx_tcp_client_socket_bind(&client_socket, 50295, 1 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;

    /* Call connect to send an SYN.  */ 
    status = nx_tcp_client_socket_connect(&client_socket, HTTP_SERVER_ADDRESS, 80, 2 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;

    /* Allocate a packet.  */
    status = nx_packet_allocate(&client_pool, &my_packet, NX_TCP_PACKET, 1 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;

    /* Write HEAD packet into the packet payload.  */
    status = nx_packet_data_append(my_packet, pkt, sizeof(pkt), &client_pool, 1 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;

    /* Send the packet out.  */
    status = nx_tcp_socket_send(&client_socket, my_packet, 1 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;

    /* Receive the response from http server. */
    status =  nx_tcp_socket_receive(&client_socket, &recv_packet, 1 * NX_IP_PERIODIC_RATE);
    if(status)
        error_counter++;
    else
    {
        buffer_ptr = (CHAR *)recv_packet ->nx_packet_prepend_ptr;

        /* Check the status, If success , it should be 200. */
        if((buffer_ptr[9] != '2') || (buffer_ptr[10] != '0') || (buffer_ptr[11] != '0'))
            error_counter++;

        nx_packet_release(recv_packet);
    }

    tx_thread_sleep(1 * NX_IP_PERIODIC_RATE);
    
    if((error_counter) || (multipart_flag != 1))
    {
        printf("ERROR!\n");
        test_control_return(1);
    }
    else
    {
        printf("SUCCESS!\n");
        test_control_return(0);
    }

}


/* Define the helper HTTP server thread.  */
void    thread_server_entry(ULONG thread_input)
{

UINT            status;

    /* Print out test information banner.  */
    printf("NetX Test:   HTTP Multipart Fragment Test..............................");

    /* Check for earlier error. */
    if(error_counter)
    {
        printf("ERROR!\n");
        test_control_return(1);
    }

    /* OK to start the HTTP Server.   */
    status = nx_http_server_start(&my_server);
    if(status)
        error_counter++;

    tx_thread_sleep(2 * NX_IP_PERIODIC_RATE);

    status = nx_http_server_delete(&my_server);
    if(status)
        error_counter++;

}

UINT    my_get_notify(NX_HTTP_SERVER *server_ptr, UINT request_type, CHAR *resource, NX_PACKET *packet_ptr)
{

ULONG      offset, total_length;
ULONG      length;
UCHAR       buffer[1440];
UINT       count = 0;
UINT       status;
NX_PACKET *response_pkt;

    /* Process multipart data. */
    if(request_type == NX_HTTP_SERVER_POST_REQUEST)
    {
        while(nx_http_server_get_entity_header(server_ptr, &packet_ptr, buffer, sizeof(buffer)) == NX_SUCCESS)
        {

            multipart_flag = 1;

            /* Send the result. */
            total_length = 0;
            count++;

            while(nx_http_server_get_entity_content(server_ptr, &packet_ptr, &offset, &length) == NX_SUCCESS)
            {
                /* Print content data. */
                nx_packet_data_extract_offset(packet_ptr, offset, buffer, length, &length);
                buffer[length] = 0;
                total_length += length;
            }
            
            /* Check the data length. */
            if((count == 1) && (total_length != 196))
                error_counter++;
            else if((count == 2) && (total_length != 175))
                error_counter++;
            else if((count == 3) && (total_length != 195))
                error_counter++;
            else if((count == 4) && (total_length != 205))
                error_counter++;
            else if((count == 5) && (total_length != 220))
                error_counter++;
            
        }

        /* Generate HTTP header. */
        status = nx_http_server_callback_generate_response_header(server_ptr, &response_pkt, NX_HTTP_STATUS_OK, 800, "text/html", "Server: NetXDuo HTTP 5.3\r\n");
        if(status == NX_SUCCESS)
        {
            if(nx_http_server_callback_packet_send(server_ptr, response_pkt))
                nx_packet_release(response_pkt);
        }
        else
            error_counter++;
    }
    else
        return NX_SUCCESS;

    return(NX_HTTP_CALLBACK_COMPLETED);
}
#else

#ifdef CTEST
VOID test_application_define(void *first_unused_memory)
#else
void    netx_http_multipart_fragment_test_application_define(void *first_unused_memory)
#endif
{

    printf("NetX Test:   HTTP Multipart Fragment Test..............................N/A\n");
    test_control_return(3);
}
#endif

