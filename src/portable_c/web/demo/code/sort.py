"""Sort the made-up cities of ../data/cities.csv by their population."""
import csv

with open("../data/cities.csv", newline="") as file:
    rows = list(csv.DictReader(file))
for row in sorted(rows, key=lambda row: int(row["population"]), reverse=True):
    print(f"{row['city']:<12} {int(row['population']):>10,}")
