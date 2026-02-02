import sqlite3
from contextlib import contextmanager

@contextmanager
def get_db():
    conn = sqlite3.connect('tasks.db')
    conn.row_factory = sqlite3.Row
    try:
        yield conn
        conn.commit()
    except Exception:
        conn.rollback()
        raise
    finally:
        conn.close()

def init_db():
    with get_db() as conn:
        conn.execute('''
            CREATE TABLE IF NOT EXISTS tasks (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                title TEXT NOT NULL,
                completed INTEGER DEFAULT 0
            )
        ''')

# CREATE
def create_task(title):
    with get_db() as conn:
        cursor = conn.execute('INSERT INTO tasks (title) VALUES (?)', (title,))
        return cursor.lastrowid

# READ
def get_all_tasks():
    with get_db() as conn:
        return [dict(row) for row in conn.execute('SELECT * FROM tasks')]

def get_task(task_id):
    with get_db() as conn:
        row = conn.execute('SELECT * FROM tasks WHERE id = ?', (task_id,)).fetchone()
        return dict(row) if row else None

# UPDATE
def update_task(task_id, title=None, completed=None):
    updates = []
    params = []
    if title is not None:
        updates.append('title = ?')
        params.append(title)
    if completed is not None:
        updates.append('completed = ?')
        params.append(completed)
    if not updates:
        return False
    params.append(task_id)
    with get_db() as conn:
        conn.execute(f'UPDATE tasks SET {", ".join(updates)} WHERE id = ?', params)
        return conn.total_changes > 0

# DELETE
def delete_task(task_id):
    with get_db() as conn:
        conn.execute('DELETE FROM tasks WHERE id = ?', (task_id,))
        return conn.total_changes > 0

# Example usage
if __name__ == '__main__':
    init_db()
    
    # Create
    task_id = create_task('Learn Python')
    print(f'Created task with id: {task_id}')
    
    # Read
    tasks = get_all_tasks()
    print(f'All tasks: {tasks}')
    
    # Update
    update_task(task_id, completed=1)
    task = get_task(task_id)
    print(f'Updated task: {task}')
    
    # Delete
    delete_task(task_id)
    print('Task deleted')
