using Microsoft.AspNetCore.Mvc;
using TodoApp.Models;

namespace TodoApp.Controllers;

public class TodoController : Controller
{
    private static readonly List<TodoItem> Items =
    [
        new TodoItem { Id = 1, Title = "Invata ASP.NET Core MVC" },
        new TodoItem { Id = 2, Title = "Creeaza prima sarcina TODO" }
    ];

    private static int nextId = 3;

    public IActionResult Index()
    {
        return View(Items.OrderBy(item => item.IsDone).ThenByDescending(item => item.CreatedAt));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Add(string title)
    {
        if (!string.IsNullOrWhiteSpace(title))
        {
            Items.Add(new TodoItem
            {
                Id = nextId++,
                Title = title.Trim()
            });
        }

        return RedirectToAction(nameof(Index));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Toggle(int id)
    {
        var item = Items.FirstOrDefault(todo => todo.Id == id);
        if (item is not null)
        {
            item.IsDone = !item.IsDone;
        }

        return RedirectToAction(nameof(Index));
    }

    [HttpPost]
    [ValidateAntiForgeryToken]
    public IActionResult Delete(int id)
    {
        var item = Items.FirstOrDefault(todo => todo.Id == id);
        if (item is not null)
        {
            Items.Remove(item);
        }

        return RedirectToAction(nameof(Index));
    }
}
