import subprocess
import os

def automate_commits(source_file, target_file):
    # Kaynak dosyayı oku
    with open(source_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    total_lines = len(lines)
    
    # Mevcut hedef dosyayı temizle
    if os.path.exists(target_file):
        os.remove(target_file)

    current_code = []

    for i, line in enumerate(lines, 1):
        current_code.append(line)
        
        # sub1.c'ye satırı ekle
        with open(target_file, 'w', encoding='utf-8') as f:
            f.writelines(current_code)
        
        # Git komutlarını hazırla
        commit_message = f"amk {i} - `amk-test.c` (step {i}/{total_lines})"
        
        try:
            # Commit işlemini gerçekleştir
            subprocess.run(["git", "add", target_file], check=True)
            subprocess.run(["git", "commit", "-m", commit_message], check=True)
            print(f"Başarılı: {commit_message}")
        except subprocess.CalledProcessError as e:
            print(f"Hata oluştu: {e}")
            break

if __name__ == "__main__":
    # Scriptin bulunduğu dizini otomatik tespit et
    script_dir = os.path.dirname(os.path.abspath(__file__))
    
    # Dosya yollarını bu dizine göre birleştir
    source = os.path.join(script_dir, 'amk-test.txt')
    target = os.path.join(script_dir, 'amk-test.c')
    
    # Dosyaların varlığını kontrol et
    if not os.path.exists(source):
        print(f"Hata: {source} bulunamadı!")
    else:
        automate_commits(source, target)