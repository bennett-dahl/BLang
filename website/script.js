document.addEventListener("DOMContentLoaded", () => {
  // Add marked.js library for Markdown conversion
  const markedScript = document.createElement("script");
  markedScript.src = "https://cdn.jsdelivr.net/npm/marked/marked.min.js";
  document.head.appendChild(markedScript);

  markedScript.onload = () => {
    // Configure marked options
    marked.setOptions({
      gfm: true,
      breaks: true,
      highlight: function (code, lang) {
        if (lang && hljs.getLanguage(lang)) {
          try {
            return hljs.highlight(code, { language: lang }).value;
          } catch (e) {
            console.error(e);
          }
        }
        return hljs.highlightAuto(code).value;
      },
    });

    // Initialize the navigation
    initNavigation();
  };

  // Initialize navigation click events and active states
  function initNavigation() {
    const navLinks = document.querySelectorAll("#nav-tree a");

    navLinks.forEach((link) => {
      link.addEventListener("click", function (e) {
        e.preventDefault();

        // Update active state
        navLinks.forEach((l) => l.classList.remove("active"));
        this.classList.add("active");

        // Load the lesson content
        const lessonFile =
          "lessons/" + this.getAttribute("data-lesson") + ".md";
        loadLessonContent(lessonFile);
      });
    });

    // Load first lesson by default if no lesson is active
    if (!document.querySelector("#nav-tree a.active")) {
      loadLessonContent("lessons/lesson1_foundations.md");
      navLinks[0].classList.add("active");
    }
  }

  // Load lesson content from Markdown file
  function loadLessonContent(fileName) {
    fetch(fileName)
      .then((response) => {
        if (!response.ok) {
          throw new Error(`Failed to load ${fileName}`);
        }
        return response.text();
      })
      .then((markdownContent) => {
        // Convert markdown to HTML
        const htmlContent = marked.parse(markdownContent);

        // Update the content area
        document.getElementById("lesson-content").innerHTML = htmlContent;

        // Highlight code blocks
        document.querySelectorAll("pre code").forEach((block) => {
          hljs.highlightBlock(block);
        });

        // Scroll to top
        window.scrollTo(0, 0);
      })
      .catch((error) => {
        console.error("Error loading lesson content:", error);
        document.getElementById("lesson-content").innerHTML = `
                    <h2>Error Loading Content</h2>
                    <p>There was an error loading the lesson content. Please try again later.</p>
                    <p>Error details: ${error.message}</p>
                `;
      });
  }
});
