# Ble logger

Collect the addresses of nearby BLE devices

Use with bleaddrdump  
Place the `blescan` executable in the same directory

```
./logger -p <Post >
                printf("-u : Post Request URL\n");
                printf("-t : Dump Interval\n");
                printf("-f : Save File Name\n");
                printf("-p : Dump Preview\n");
                printf("-h : Help\n");
```

|Option|Note|Ex|
|:--|:--|:--|
|-u [URL]|Post Request URL|-u `https://testdomain/dump`
|-t [TIME]|Dump Interval Time|-t 10|
|-f [FILENAME]|Save File Name|-f bleaddr.log|
|-p|Dump Preview|-p|

