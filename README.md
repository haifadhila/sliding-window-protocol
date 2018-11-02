## Sliding Window Protocol
**Tugas Besar I IF3130 Jaringan Komputer**
**Lossless Data Transfer**

Anggota:
 - Yuly Haruka Berliana Gunawan / 13516031
 - Haifa Fadhila Ilma / 13516076
 - Nadija Herdwina Putri Soerojo / 13516130
	(K-01)

### Petunjuk Penggunaan Program
Pertama, lakukan *Makefile*
```

```
Cara menjalankan program setelah di-*Makefile*:
1.	Pertama, jalankan Receiver, eksekusi pada command prompt/ terminal:
	```
	./recvfile <filename> <windowsize> <buffersize> <port>
	```
	Contoh:
	```
	./recvfile "hello.txt" 10 1000 2334
	```
	Contoh di atas akan menjalankan program untuk menerima file dengan *hello.txt* sebagai file yang diterima, ukuran window sebesar 10 untuk Sliding Window Protocol, ukuran buffer 1000 (dapat memuat 1000 frame) dan port 2334.

2. Kedua, jalankan Sender, eksekusi pada command prompt/ terminal:
	```
	./sendfile <filename> <windowsize> <buffersize> <destination_ip> <destination_port> "
	```
	Contoh:
	```
	./sendfile "test.txt" 10 1000 127.0.0.1 2334
	```
	Contoh di atas akan menjalankan program untuk mengirim file dengan *test.txt* sebagai file yang akan dikirim, ukuran window sebesar 10 untuk Sliding Window Protocol, ukuran buffer 1000 (dapat memuat 1000 frame), IP Address 127.0.0.1 dan port 2334.


### Cara Kerja Program
Sliding Window Protocol merupakan salah satu metode

Sliding Window Protocol dengan Selective Repeat Automatic Repeat Request.

### Pembagian Tugas

1. Yuly Haruka Berliana Gunawan / 13516031
		- Read File
		- Write File
		- Send & Receive Frame
		- Sliding Window Protocol in Receiver
		- Serialize Ack into Ack Format

2. Haifa Fadhila Ilma / 13516076
		- Serialize Buffer Data into Frame Format
		- Checksum
		- Send & Receive Frame

2. Nadija Herdwina Putri Soerojo / 13516130
		- Sliding Window Protocol in Sender
		- Add multithreading in Sliding Window Protocol
		- Checksum
