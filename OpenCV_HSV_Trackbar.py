import cv2
import numpy as np

# Fungsi callback kosong untuk Trackbar
def nothing(x):
    pass

# Ganti dengan URL stream valid dari ESP32-CAM (biasanya port 81 + endpoint /stream)
url = "http://192.168.1.22:81/stream"
print(f"[INFO] Mencoba buka URL stream: {url}")

# Inisialisasi VideoCapture
cap = cv2.VideoCapture(url)
if not cap.isOpened():
    print("[ERROR] Tidak bisa membuka VideoCapture. Cek URL, port, dan koneksi ESP32-CAM.")
    exit()
else:
    print("[INFO] Berhasil membuka VideoCapture.")

# Buat window untuk menampilkan output dan Trackbar
cv2.namedWindow("Live Transmission", cv2.WINDOW_AUTOSIZE)
cv2.namedWindow("Tracking", cv2.WINDOW_NORMAL)
cv2.resizeWindow("Tracking", 400, 300)

# Nilai awal HSV untuk mendeteksi warna api (fire):
initial_l_h, initial_l_s, initial_l_v = 0, 0, 155
initial_u_h, initial_u_s, initial_u_v = 255, 255, 255

# Inisialisasi trackbar HSV (lower & upper) dengan nilai awal di atas
cv2.createTrackbar("LH", "Tracking", initial_l_h, 255, nothing)
cv2.createTrackbar("LS", "Tracking", initial_l_s, 255, nothing)
cv2.createTrackbar("LV", "Tracking", initial_l_v, 255, nothing)
cv2.createTrackbar("UH", "Tracking", initial_u_h, 255, nothing)
cv2.createTrackbar("US", "Tracking", initial_u_s, 255, nothing)
cv2.createTrackbar("UV", "Tracking", initial_u_v, 255, nothing)

while True:
    # Baca frame dari VideoCapture
    ret, frame = cap.read()
    if not ret:
        print("[ERROR] Gagal membaca frame. Periksa koneksi stream.")
        break

    # Konversi BGR ke HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Ambil posisi trackbar (bisa diubah-ubah secara real-time)
    l_h = cv2.getTrackbarPos("LH", "Tracking")
    l_s = cv2.getTrackbarPos("LS", "Tracking")
    l_v = cv2.getTrackbarPos("LV", "Tracking")
    u_h = cv2.getTrackbarPos("UH", "Tracking")
    u_s = cv2.getTrackbarPos("US", "Tracking")
    u_v = cv2.getTrackbarPos("UV", "Tracking")

    # Buat array batas bawah dan atas HSV
    lower_bound = np.array([l_h, l_s, l_v])
    upper_bound = np.array([u_h, u_s, u_v])

    # Hitung mask (hanya pixel di rentang HSV yang akan putih)
    mask = cv2.inRange(hsv, lower_bound, upper_bound)

    # Cari kontur pada mask
    cnts, _ = cv2.findContours(mask, cv2.RETR_TREE, cv2.CHAIN_APPROX_SIMPLE)

    # Loop setiap kontur dan gambar hanya yang area-nya cukup besar
    for c in cnts:
        area = cv2.contourArea(c)
        if area > 1000:  # Threshold area (ubah jika perlu)
            # Gambar kontur dengan warna biru, ketebalan 3 px
            cv2.drawContours(frame, [c], -1, (255, 0, 0), 3)

            # Hitung centroid
            M = cv2.moments(c)
            if M["m00"] != 0:
                cx = int(M["m10"] / M["m00"])
                cy = int(M["m01"] / M["m00"])
            else:
                cx, cy = 0, 0

            # Gambar lingkaran putih di centroid
            cv2.circle(frame, (cx, cy), 7, (255, 255, 255), -1)
            # Tulis label "fire" berwarna merah dekat centroid
            cv2.putText(
                frame,
                "fire",
                (cx - 20, cy - 20),
                cv2.FONT_HERSHEY_SIMPLEX,
                1,
                (0, 0, 255),
                2
            )

    # Hasil bitwise-and (hanya area fire berwarna)
    res = cv2.bitwise_and(frame, frame, mask=mask)

    # Tampilkan window
    cv2.imshow("Live Transmission", frame)  # Frame asli + kontur + centroid + label "fire"
    cv2.imshow("Mask", mask)                # Binary mask (area white = rentang HSV)
    cv2.imshow("Result", res)               # Hanya area yang ter-mask

    # Tekan 'q' untuk keluar
    if cv2.waitKey(1) & 0xFF == ord('q'):
        print("[INFO] Keluar oleh user.")
        break

# Bersihkan
cap.release()
cv2.destroyAllWindows()
print("[INFO] Program selesai.")