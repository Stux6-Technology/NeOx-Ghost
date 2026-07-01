import subprocess
import os
import random
from datetime import datetime, timedelta

def git_commit_history(repo_path, source_file, target_file):
    # Proje dizinine geçiş yap
    os.chdir(repo_path)
    
    start_date = datetime(2026, 1, 1)
    end_date = datetime(2026, 4, 4)
    delta = end_date - start_date

    # Kaynak dosyayı oku
    with open(source_file, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    # Toplam commit sayısını önceden hesapla (X/XXXX formatı için)
    daily_counts = []
    total_commits_planned = 0
    for i in range(delta.days + 1):
        count = random.randint(1, 99)
        daily_counts.append(count)
        total_commits_planned += count

    current_line_index = 0
    kernel_counter = 1 # Global sayaç (1'den başlar, her adımda artar)

    for i in range(delta.days + 1):
        current_day = start_date + timedelta(days=i)
        commit_count = daily_counts[i]
        
        print(f"Tarih: {current_day.strftime('%Y-%m-%d')} - {commit_count} commit atılıyor...")

        for _ in range(commit_count):
            if current_line_index < len(lines):
                # Kod ekleme mantığı
                with open(target_file, 'a', encoding='utf-8') as f:
                    f.write(lines[current_line_index])
                current_line_index += 1
            
            # Rastgele saat (09:00 - 23:59 arası)
            random_hour = random.randint(9, 23)
            random_minute = random.randint(0, 59)
            random_second = random.randint(0, 59)
            commit_time = current_day.replace(hour=random_hour, minute=random_minute, second=random_second)
            date_str = commit_time.isoformat()

            # Format: kernel X - `main scripts` (X/XXXX)
            commit_message = f"kernel {kernel_counter} - `main scripts` ({kernel_counter}/{total_commits_planned})"

            # Git ortam değişkenlerini ayarla (Geçmiş tarihli commit için)
            env = os.environ.copy()
            env['GIT_AUTHOR_DATE'] = date_str
            env['GIT_COMMITTER_DATE'] = date_str

            subprocess.run(["git", "add", target_file], env=env)
            subprocess.run(["git", "commit", "-m", commit_message], env=env)
            
            # Global sayacı bir sonraki commit için artır
            kernel_counter += 1

    print(f"İşlem tamamlandı! Toplam {kernel_counter - 1} commit oluşturuldu.")

if __name__ == "__main__":
    # Kendi dizin yolunu buraya yazmalısın
    repo = "/Users/fero/Documents/Git/NeOx/NeOx-Ghost/"
    git_commit_history(repo, 'test/test.txt', 'test/test.py')