'use client';

import { useState } from 'react';

const BACKEND_URL = process.env.NEXT_PUBLIC_BACKEND_URL || 'http://localhost:8080';

export default function Home() {
  // Form state
  const [form, setForm] = useState({
    name: '',
    age: '',
    weight: '',
    height: '',
    goal: 'balance',
    restrictions: '',
  });

  // Loading and response states
  const [loading, setLoading] = useState(false);
  const [userId, setUserId] = useState<number | null>(null);
  const [plan, setPlan] = useState<any>(null);
  const [score, setScore] = useState<number | null>(null);
  const [shoppingList, setShoppingList] = useState<any>(null);
  const [error, setError] = useState<string | null>(null);
  const [success, setSuccess] = useState<string | null>(null);

  // Handle form input change
  const handleChange = (e: React.ChangeEvent<HTMLInputElement | HTMLTextAreaElement | HTMLSelectElement>) => {
    const { name, value } = e.target;
    setForm(prev => ({ ...prev, [name]: value }));
  };

  // Handle form submit
  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setError(null);
    setSuccess(null);
    setLoading(true);

    try {
      // Validate required fields
      if (!form.name || !form.age || !form.weight || !form.height) {
        throw new Error('Please fill in all required fields');
      }

      // Send profile to backend
      const profileRes = await fetch(`${BACKEND_URL}/api/profile`, {
        method: 'POST',
        headers: {
          'Content-Type': 'application/json',
        },
        body: JSON.stringify({
          name: form.name,
          age: parseInt(form.age),
          weight: parseFloat(form.weight),
          height: parseFloat(form.height),
          goal: form.goal,
          restrictions: form.restrictions.split(',').map((r: string) => r.trim()).filter((r: string) => r.length > 0),
        }),
      });

      if (!profileRes.ok) {
        const errorData = await profileRes.json();
        throw new Error(errorData.message || 'Failed to save profile');
      }

      const profileData = await profileRes.json();
      setUserId(profileData.data.user_id);
      setSuccess('Profile saved successfully!');

      // Fetch meal plan
      const planRes = await fetch(`${BACKEND_URL}/api/plan?user_id=${profileData.data.user_id}`);
      if (!planRes.ok) {
        throw new Error('Failed to generate meal plan');
      }
      const planData = await planRes.json();
      setPlan(planData.data.plan);
      setScore(planData.data.score);

    } catch (err: any) {
      setError(err.message || 'An unknown error occurred');
    } finally {
      setLoading(false);
    }
  };

  // Handle shopping list generation
  const handleShoppingList = async () => {
    if (!userId) return;
    setError(null);
    try {
      const res = await fetch(`${BACKEND_URL}/api/shopping-list?user_id=${userId}`);
      if (!res.ok) throw new Error('Failed to generate shopping list');
      const data = await res.json();
      setShoppingList(data.data.shopping_list);
    } catch (err: any) {
      setError(err.message || 'An unknown error occurred');
    }
  };

  // Reset form and state
  const reset = () => {
    setForm({
      name: '',
      age: '',
      weight: '',
      height: '',
      goal: 'balance',
      restrictions: '',
    });
    setUserId(null);
    setPlan(null);
    setScore(null);
    setShoppingList(null);
    setError(null);
    setSuccess(null);
  };

  return (
    <div className="min-h-screen flex flex-col flex-1 items-center justify-center bg-zinc-50 font-sans dark:bg-black">
      <main className="flex flex-1 w-full max-w-2xl flex-col items-center justify-between py-12 px-4 sm:items-start sm:px-6 lg:px-8">
        {/* Form Section */}
        {!userId && (
          <div className="w-full space-y-6">
            <h1 className="text-2xl font-bold text-center text-black dark:text-zinc-50">
              AI Food Planner
            </h1>
            <p className="text-center text-zinc-600 dark:text-zinc-400 max-w-md">
              Get personalized meal plans based on your profile and goals.
            </p>

            {/* Success/Error Messages */}
            {success && (
              <div className="px-4 py-3 mb-4 text-sm font-medium text-center text-green-800 bg-green-50 dark:text-green-400 dark:bg-green-900/20 rounded-lg">
                {success}
              </div>
            )}
            {error && (
              <div className="px-4 py-3 mb-4 text-sm font-medium text-center text-red-800 bg-red-50 dark:text-red-400 dark:bg-red-900/20 rounded-lg">
                {error}
              </div>
            )}

            <form onSubmit={handleSubmit} className="space-y-6">
              <div className="grid gap-4 sm:grid-cols-2">
                <div>
                  <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">Name</label>
                  <input
                    name="name"
                    value={form.name}
                    onChange={handleChange}
                    className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                    required
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">Age</label>
                  <input
                    name="age"
                    type="number"
                    value={form.age}
                    onChange={handleChange}
                    className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                    min="1"
                    required
                  />
                </div>
              </div>

              <div className="grid gap-4 sm:grid-cols-2">
                <div>
                  <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">Weight (kg)</label>
                  <input
                    name="weight"
                    type="number"
                    value={form.weight}
                    onChange={handleChange}
                    className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                    min="0.1"
                    step="0.1"
                    required
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">Height (cm)</label>
                  <input
                    name="height"
                    type="number"
                    value={form.height}
                    onChange={handleChange}
                    className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                    min="1"
                    required
                  />
                </div>
              </div>

              <div>
                <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">Goal</label>
                <select
                  name="goal"
                  value={form.goal}
                  onChange={handleChange}
                  className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                >
                  <option value="balance">Balance</option>
                  <option value="weight_loss">Weight Loss</option>
                  <option value="muscle_gain">Muscle Gain</option>
                </select>
              </div>

              <div>
                <label className="block text-sm font-medium text-zinc-700 dark:text-zinc-300 mb-1">
                  Dietary Restrictions (comma-separated)
                </label>
                <input
                  name="restrictions"
                  value={form.restrictions}
                  onChange={handleChange}
                  className="w-full px-4 py-2 border border-zinc-300 rounded-md focus:outline-none focus:ring-2 focus:ring-indigo-500 dark:bg-zinc-800 dark:border-zinc-700 dark:text-zinc-100"
                  placeholder="e.g., vegetarian, gluten-free, dairy-free"
                />
              </div>

              <button
                type="submit"
                disabled={loading}
                className="w-full px-4 py-3 bg-indigo-600 text-white font-medium rounded-md hover:bg-indigo-700 focus:outline-none focus:ring-2 focus:ring-indigo-500 focus:ring-offset-2 disabled:opacity-50 transition-colors"
              >
                {loading ? 'Generating...' : 'Get Meal Plan'}
              </button>
            </form>

            {userId && (
              <button
                onClick={reset}
                className="mt-4 px-4 py-2 text-sm font-medium text-indigo-600 hover:text-indigo-500"
              >
                Start Over
              </button>
            )}
          </div>
        )}

        {/* Meal Plan Section */}
        {userId && plan && (
          <div className="w-full space-y-8">
            <div className="flex flex-col items-center">
              <h2 className="text-2xl font-bold text-black dark:text-zinc-50">
                Your 7-Day Meal Plan
              </h2>
              {score !== null && (
                <div className="mt-2 flex items-center space-x-3">
                  <div className="flex items-center justify-center w-10 h-10 rounded-full bg-indigo-100 text-indigo-600">
                    {score}
                  </div>
                  <p className="text-zinc-600 dark:text-zinc-400">Nutrition Score: {score}/100</p>
                </div>
              )}
            </div>

            {/* Shopping List Button */}
            <div className="flex justify-center">
              <button
                onClick={handleShoppingList}
                disabled={loading}
                className="px-6 py-3 bg-green-600 text-white font-medium rounded-md hover:bg-green-700 focus:outline-none focus:ring-2 focus:ring-green-500 focus:ring-offset-2 transition-colors"
              >
                {shoppingList ? 'View Shopping List' : 'Generate Shopping List'}
              </button>
            </div>

            {/* Meal Plan Details */}
            <div className="space-y-6">
              {plan.days && plan.days.map((day: any, index: number) => (
                <div key={index} className="border border-zinc-200 rounded-lg dark:border-zinc-700">
                  <div className="px-4 py-3 bg-zinc-50 dark:bg-zinc-900/50">
                    <h3 className="text-lg font-semibold text-black dark:text-zinc-50">Day {day.day}</h3>
                  </div>
                  <div className="px-4 py-4 space-y-4">
                    {/* Meals */}
                    {[
                      ['breakfast', 'Breakfast'],
                      ['lunch', 'Lunch'],
                      ['dinner', 'Dinner'],
                      ['snack', 'Snack'],
                    ].map((meal) => {
                      const [key, label] = meal;
                      return (
                        <div key={key} className="border-t pt-4 first:border-t-0">
                          <h4 className="font-medium text-zinc-700 dark:text-zinc-300">{label}</h4>
                          {plan[key] && (
                                            <div className="mt-2 space-y-2">
                    <div className="flex items-center space-x-2">
                      <span className="flex items-center justify-center w-5 h-5 bg-indigo-100 text-indigo-600 rounded-full text-xs">
                        🔥
                      </span>
                      <span className="text-zinc-600 dark:text-zinc-400">
                        Calories: {plan[key].calories || 0}
                      </span>
                    </div>
                    <div className="flex items-center space-x-2">
                      <span className="flex items-center justify-center w-5 h-5 bg-indigo-100 text-indigo-600 rounded-full text-xs">
                        🥩
                      </span>
                      <span className="text-zinc-600 dark:text-zinc-400">
                        Proteins: {plan[key].proteins || 0}g
                      </span>
                    </div>
                    <div className="flex items-center space-x-2">
                      <span className="flex items-center justify-center w-5 h-5 bg-indigo-100 text-indigo-600 rounded-full text-xs">
                        🍞
                      </span>
                      <span className="text-zinc-600 dark:text-zinc-400">
                        Carbs: {plan[key].carbs || 0}g
                      </span>
                    </div>
                    <div className="flex items-center space-x-2">
                      <span className="flex items-center justify-center w-5 h-5 bg-indigo-100 text-indigo-600 rounded-full text-xs">
                        🥑
                      </span>
                      <span className="text-zinc-600 dark:text-zinc-400">
                        Fats: {plan[key].fats || 0}g
                      </span>
                    </div>
                  </div>
                          )}
                          {!plan[key] && (
                            <p className="text-zinc-500 italic">No {label.toLowerCase()} planned</p>
                          )}
                        </div>
                      )
                    })}
                  </div>
                </div>
              ))}
            </div>
          </div>
        )}

        {/* Shopping List Section */}
        {shoppingList && (
          <div className="w-full space-y-6">
            <h2 className="text-2xl font-bold text-black dark:text-zinc-50">
              Shopping List
            </h2>
            <div className="space-y-4">
              {Object.keys(shoppingList).map((category: string) => (
                <div key={category} className="border border-zinc-200 rounded-lg dark:border-zinc-700">
                  <div className="px-4 py-3 bg-zinc-50 dark:bg-zinc-900/50">
                    <h3 className="text-lg font-semibold text-black dark:text-zinc-50">
                      {category.charAt(0).toUpperCase() + category.slice(1)}
                    </h3>
                  </div>
                  <div className="px-4 py-4">
                    <ul className="space-y-1">
                      {shoppingList[category].map((item: string, index: number) => (
                        <li key={index} className="flex items-center">
                          <span className="flex items-center justify-center w-3 h-3 bg-indigo-600 text-white rounded-full text-xs mr-2">
                            •
                          </span>
                          <span className="text-zinc-700 dark:text-zinc-300">{item}</span>
                        </li>
                      ))}
                    </ul>
                  </div>
                </div>
              ))}
            </div>
            <div className="flex justify-center">
              <button
                onClick={() => setShoppingList(null)}
                className="px-4 py-2 text-sm font-medium text-indigo-600 hover:text-indigo-500"
              >
                Close Shopping List
              </button>
            </div>
          </div>
        )}
      </main>
    </div>
  );
}