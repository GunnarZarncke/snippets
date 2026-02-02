#!/usr/bin/env python3
"""
Minimal Library Management System CLI using argparse.
Demonstrates CRUD operations for books with command-line interface.
"""
import argparse
import json
import os
from pathlib import Path

DATA_FILE = Path.home() / '.library_data.json'

def load_data():
    """Load library data from JSON file."""
    if DATA_FILE.exists():
        with open(DATA_FILE, 'r') as f:
            return json.load(f)
    return {'books': [], 'borrowed': {}}

def save_data(data):
    """Save library data to JSON file."""
    with open(DATA_FILE, 'w') as f:
        json.dump(data, f, indent=2)

def add_book(title, author, isbn=None):
    """Add a new book to the library."""
    data = load_data()
    book = {
        'id': len(data['books']) + 1,
        'title': title,
        'author': author,
        'isbn': isbn or f'ISBN-{len(data["books"]) + 1:04d}',
        'available': True
    }
    data['books'].append(book)
    save_data(data)
    print(f"Added book: {title} by {author} (ID: {book['id']})")
    return book['id']

def list_books(available_only=False):
    """List all books in the library."""
    data = load_data()
    books = data['books']
    
    if available_only:
        books = [b for b in books if b['available']]
    
    if not books:
        print("No books found.")
        return
    
    print(f"\n{'ID':<5} {'Title':<30} {'Author':<25} {'ISBN':<15} {'Status':<10}")
    print("-" * 85)
    for book in books:
        status = "Available" if book['available'] else "Borrowed"
        print(f"{book['id']:<5} {book['title']:<30} {book['author']:<25} "
              f"{book['isbn']:<15} {status:<10}")

def borrow_book(book_id, borrower):
    """Borrow a book from the library."""
    data = load_data()
    book = next((b for b in data['books'] if b['id'] == book_id), None)
    
    if not book:
        print(f"Error: Book with ID {book_id} not found.")
        return
    
    if not book['available']:
        print(f"Error: Book '{book['title']}' is already borrowed.")
        return
    
    book['available'] = False
    data['borrowed'][str(book_id)] = borrower
    save_data(data)
    print(f"'{book['title']}' borrowed by {borrower}")

def return_book(book_id):
    """Return a borrowed book to the library."""
    data = load_data()
    book = next((b for b in data['books'] if b['id'] == book_id), None)
    
    if not book:
        print(f"Error: Book with ID {book_id} not found.")
        return
    
    if book['available']:
        print(f"Error: Book '{book['title']}' is already available.")
        return
    
    borrower = data['borrowed'].pop(str(book_id), 'Unknown')
    book['available'] = True
    save_data(data)
    print(f"'{book['title']}' returned by {borrower}")

def remove_book(book_id):
    """Remove a book from the library."""
    data = load_data()
    book = next((b for b in data['books'] if b['id'] == book_id), None)
    
    if not book:
        print(f"Error: Book with ID {book_id} not found.")
        return
    
    if not book['available']:
        print(f"Error: Cannot remove '{book['title']}' - it is currently borrowed.")
        return
    
    data['books'] = [b for b in data['books'] if b['id'] != book_id]
    save_data(data)
    print(f"Removed book: '{book['title']}'")

def search_books(query):
    """Search for books by title or author."""
    data = load_data()
    query_lower = query.lower()
    matches = [
        b for b in data['books']
        if query_lower in b['title'].lower() or query_lower in b['author'].lower()
    ]
    
    if not matches:
        print(f"No books found matching '{query}'")
        return
    
    print(f"\nFound {len(matches)} book(s) matching '{query}':")
    print(f"{'ID':<5} {'Title':<30} {'Author':<25} {'Status':<10}")
    print("-" * 70)
    for book in matches:
        status = "Available" if book['available'] else "Borrowed"
        print(f"{book['id']:<5} {book['title']:<30} {book['author']:<25} {status:<10}")

def main():
    parser = argparse.ArgumentParser(
        description='Library Management System CLI',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  %(prog)s add "The Great Gatsby" "F. Scott Fitzgerald"
  %(prog)s list
  %(prog)s borrow 1 "John Doe"
  %(prog)s return 1
  %(prog)s search "Gatsby"
  %(prog)s remove 1
        """
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Available commands')
    
    # Add command
    add_parser = subparsers.add_parser('add', help='Add a new book')
    add_parser.add_argument('title', help='Book title')
    add_parser.add_argument('author', help='Book author')
    add_parser.add_argument('--isbn', help='ISBN number (optional)')
    
    # List command
    list_parser = subparsers.add_parser('list', help='List all books')
    list_parser.add_argument('--available', action='store_true', 
                            help='Show only available books')
    
    # Borrow command
    borrow_parser = subparsers.add_parser('borrow', help='Borrow a book')
    borrow_parser.add_argument('book_id', type=int, help='Book ID')
    borrow_parser.add_argument('borrower', help='Borrower name')
    
    # Return command
    return_parser = subparsers.add_parser('return', help='Return a borrowed book')
    return_parser.add_argument('book_id', type=int, help='Book ID')
    
    # Remove command
    remove_parser = subparsers.add_parser('remove', help='Remove a book')
    remove_parser.add_argument('book_id', type=int, help='Book ID')
    
    # Search command
    search_parser = subparsers.add_parser('search', help='Search for books')
    search_parser.add_argument('query', help='Search term (title or author)')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return
    
    if args.command == 'add':
        add_book(args.title, args.author, args.isbn)
    elif args.command == 'list':
        list_books(available_only=args.available)
    elif args.command == 'borrow':
        borrow_book(args.book_id, args.borrower)
    elif args.command == 'return':
        return_book(args.book_id)
    elif args.command == 'remove':
        remove_book(args.book_id)
    elif args.command == 'search':
        search_books(args.query)

if __name__ == '__main__':
    main()
