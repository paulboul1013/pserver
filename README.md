# udp server & client

# 程式說明
server中有html相關檔案的話，client可以輸入html檔案名搜尋html檔，server就可以回傳html檔案內容，並且可以自動開啟。
>linux版本需安裝xdg-open，安裝指令如下

ubuntu version
```c=
sudo apt-get install --reinstall xdg-utils
```

# 使用範例
server:
```c=
./server
```
client:
```c=
./client 127.0.0.1 index.html
```
