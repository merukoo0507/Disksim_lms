# Disksim_lms
Simple way to control and modify the source code of disksim.
    
## Reference
* http://cighao.com/2016/03/23/disksim-with-ssdmodel-source-analysis-012-write-requests/
* 孟軒's program (Disksim_lms 我有上傳到實驗室NAS)
* 編譯環境參考培堯[Accusim-Strace](https://github.com/baconYao/Accusim-Strace/blob/master/README.md)
    
## Environment
* Ubuntu 14.04 x86 (!!)
* 註: 安裝gcc後，編譯出現錯誤 fatal error: sys/types.h: No such file or directory
    * 嘗試输入`sudo apt-get install gcc-multilib`
    
## 編譯
*   到 /libparam 開啟檔案權限

        sudo chmod -f 777 *
*   到 /libddbg、/libparam、/diskmodel 編譯
        
        make clean
        make
*   到 /src 編譯

        cd src
        make clean
        make
    
## 執行
*   參數含意
    * SSD參數: `/home/merukoo/Disksim_lms/ssdmodel/valid/Intel_toolkit.parv` 
    * HDD參數:　`/home/merukoo/Disksim_lms/valid/maxtor146g/maxtor146g.parv`
    * 隨意取: `ssd_output`, `hdd_output`
```
sudo ./lmssim '/home/merukoo/Disksim_lms/ssdmodel/valid/Intel_toolkit.parv' ssd_output '/home/merukoo/Disksim_lms/valid/maxtor146g/maxtor146g.parv' hdd_output
```

## 主要修改的地方
* [lmssim.c](src/lmssim.c)
程式碼有註解，可以在以下部分做簡單操作
        
* lmssim.c:477 看帶入參數對不對
* lmssim.c:513 輸入disksim的一筆資料
    * `char buffer[] = "0 0 8 16 0";` 
    * //arrival time, device number = 0, block num, request size, read/write
* lmssim.c:531 Send_To_HDD
* lmssim.c:544 Send_To_SSD
        
-----

# Disksim
This directory contains release 4.0 of the DiskSim storage subsystem
simulator.  Check out the doc directory for he corresponding reference 
manual that describes the simulator and how to use it.

See the file COPYING for the copyright notice and copying conditions.

To quickly verify that you've got everything and that it works:

  1. "make" the top-level directory.
    
  2. "cd" to the subdirectory called "valid".

  3. type "runvalid".  This will execute the simulator a number of times, using
     sample configurations and workloads.  Among them are example validation
     experiments for a number of different SCSI disk drives.  To
     verify that things are working correctly, compare the result
     values from each execution to the expected value (provided on the
     preceding line), which is rounded.

If you plan to use disksim as a stand-alone simulator, these examples and
the user manual should get you started.

If you plan to incorporate disksim into a larger-scale simulator (e.g., a
full system simulator), disksim_interface.c should be very helpful in
getting it to happen quickly and relatively painlessly.  It is not compiled
into disksim for standalone operation.  Thanks to Eran Gabber at Lucent,
there is now a simple example of a system-level simulator incorporating
disksim as a slave -- check out syssim* (before and after compilation).

Please send bug reports, experiences, and problems to disksim@ece.cmu.edu.
If you find disksim useful, please let us know about it!

There are two public mailing lists for disksim:
disksim-announce@ece.cmu.edu
disksim-users@ece.cmu.edu

disksim-announce only receives official announcements about bugfixes
and new versions of DiskSim.  disksim-users is for public discussion.

Please visit one of these sites to join the mailing lists:

https://sos.ece.cmu.edu/mailman/listinfo/disksim-announce
https://sos.ece.cmu.edu/mailman/listinfo/disksim-users
