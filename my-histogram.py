import sys
import os
import stat
import csv
import subprocess

def main():
    # print command line arguments
    if len(sys.argv) != 2:
        sys.stderr.write("Error: should have 1 argument")
    directory = sys.argv[1]

    counts = {"regular":0, 'directory':0, 'link':0, 'fifo':0, 'socket':0, 'block':0, 'character':0}
    
    
    for root, dirs, files in os.walk(directory):
        counts['directory'] += len(dirs)
        print(dirs)
        for file_ in files:
            file_path = os.path.join(root, file_)
            mode = os.stat(file_path).st_mode
            if stat.S_ISDIR(mode):
                counts['directory'] += 1
            elif stat.S_ISREG(mode):
                counts['regular'] += 1
            elif stat.S_ISSOCK(mode):
                counts['socket'] += 1
            elif stat.S_ISFIFO(mode):
                counts['fifo'] += 1
            elif stat.S_ISLNK(mode):
                counts['link'] += 1
            elif stat.S_ISBLK(mode):
                counts['block'] += 1
            elif stat.S_ISCHR(mode):
                counts['charachter'] += 1
    # for entry in os.scandir(directory):
    #     mode = os.stat(entry).st_mode
    #     if stat.S_ISDIR(mode):
    #         counts['directory'] += 1
    #     elif stat.S_ISREG(mode):
    #         counts['regular'] += 1
    #     elif stat.S_ISSOCK(mode):
    #         counts['socket'] += 1
    #     elif stat.S_ISFIFO(mode):
    #         counts['fifo'] += 1
    #     elif stat.S_ISLNK(mode):
    #         counts['link'] += 1
    #     elif stat.S_ISBLK(mode):
    #         counts['block'] += 1
    #     elif stat.S_ISCHR(mode):
    #         counts['charachter'] += 1
        
    with open('data.csv', 'w') as csvfile:
        csvfile.truncate()
        filewriter = csv.writer(csvfile,delimiter=',',quotechar='|',quoting=csv.QUOTE_MINIMAL)
        for file_type in counts:
            filewriter.writerow([file_type, str(counts[file_type])])
    run_gnu_plot()

def run_gnu_plot():
    plot = subprocess.Popen(
        ['gnuplot'], 
        shell=True,
        stdin=subprocess.PIPE, 
        encoding='utf8')
    plot.communicate("""
    set datafile separator ','
    set terminal png truecolor
    set output "histogram.png"
    set grid
    set xtics rotate by 90
    set style data histograms
    set style fill solid 1.00 border -1
    plot "data.csv"  using 2:xtic(1) title "frequency"
    """)

if __name__ == "__main__":
    main()